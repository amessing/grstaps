#include "experiments.hpp"

// external
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>

// grstaps
#include <grstaps/Search/SearchResultPackager.h>
#include <grstaps/Task_Allocation/AllocationDistance.h>
#include <grstaps/Task_Allocation/AllocationExpander.h>
#include <grstaps/Task_Allocation/AllocationIsGoal.h>
#include <grstaps/Task_Allocation/checkAllocatable.h>
#include <grstaps/json_conversions.hpp>
#include <grstaps/logger.hpp>

namespace grstaps
{
    namespace icra2021
    {
        Experiments& Experiments::getInstance()
        {
            static Experiments instance;
            return instance;
        }

        nlohmann::json Experiments::solve(const unsigned int problem_number, const float alpha)
        {
            m_problem_number = problem_number;
            m_sequential     = false;
            m_alpha          = alpha;
            Timer::setITAGS();
            IcraProblem problem = getProblem(problem_number);
            Logger::info("Solving with alpha={}", alpha);

            // Also can any of them be const? That will help with multithreading in the future (fewer mutexes)
            auto heuristic = boost::make_shared<const TAGoalDist>(alpha);
            auto path_cost = boost::make_shared<const TAScheduleTime>();

            auto isGoal   = boost::make_shared<const AllocationIsGoal>();
            auto expander = boost::make_shared<const AllocationExpander>(heuristic, path_cost);
            SearchResultPackager<TaskAllocation> package;

            taskAllocationToScheduling taToSched(problem.motionPlanners(), problem.startingLocations().get());
            taToSched.setActionLocations(problem.actionLocations());

            auto numSpec = boost::make_shared<std::vector<int>>(problem.robotTraits()->size(), 1);

            m_timer.start();

            TaskAllocation ta(false,  // usingSpecies
                              problem.goalDistribution(),
                              problem.robotTraits().get(),
                              problem.noncumTraitCutoff(),
                              taToSched,
                              problem.durations(),
                              problem.orderingConstraints(),
                              numSpec,
                              problem.speedIndex(),
                              problem.mpIndex());

            auto root = boost::make_shared<Node<TaskAllocation>>(ta.getID(), ta);
            root->setData(ta);
            Graph<TaskAllocation> allocationGraph;
            allocationGraph.addNode(root);
            m_search = std::make_unique<AStarSearch<TaskAllocation>>(allocationGraph, root);

            m_search->search(isGoal, expander, &package);
            m_timer.recordSplit(Timer::SplitType::e_ta);
            m_timer.stop();

            nlohmann::json metrics = {
                {"solved", package.foundGoal},
                {"makespan", package.foundGoal ? package.finalNode->getData().getScheduleTime() : -1},
                {"nodes_expanded", m_search->nodesExpanded},
                {"nodes_visited", m_search->nodesSearched},
                {"Timer", m_timer}};

            return metrics;
        }
        nlohmann::json Experiments::solveSequentially(const unsigned int problem_number)
        {
            m_problem_number = problem_number;
            m_sequential     = true;
            m_alpha          = 0.0f;
            Timer::setITAGS_S();
            IcraProblem problem = getProblem(problem_number);

            Logger::info("Solving sequentially");
            // Also can any of them be const? That will help with multithreading in the future (fewer mutexes)
            auto heuristic = boost::make_shared<const AllocationDistance>();
            auto path_cost = boost::make_shared<const TAScheduleTime>();

            auto isGoal   = boost::make_shared<const AllocationIsGoal>();
            auto expander = boost::make_shared<const AllocationExpander>(heuristic, path_cost);
            SearchResultPackager<TaskAllocation> package;

            taskAllocationToScheduling taToSched(problem.motionPlanners(), problem.startingLocations().get());
            taToSched.setActionLocations(problem.actionLocations());

            auto numSpec = boost::make_shared<std::vector<int>>(problem.robotTraits()->size(), 1);

            m_timer.start();

            TaskAllocation ta(false,  // usingSpecies
                              problem.goalDistribution(),
                              problem.robotTraits().get(),
                              problem.noncumTraitCutoff(),
                              taToSched,
                              problem.durations(),
                              problem.orderingConstraints(),
                              numSpec,
                              problem.speedIndex(),
                              problem.mpIndex());

            auto root = boost::make_shared<Node<TaskAllocation>>(ta.getID(), ta);
            root->setData(ta);
            Graph<TaskAllocation> allocationGraph;
            allocationGraph.addNode(root);

            m_search = std::make_unique<AStarSearch<TaskAllocation>>(allocationGraph, root);
            do
            {
                m_search->search(isGoal, expander, &package);
                if(package.foundGoal)
                {
                    m_timer.recordSplit(Timer::SplitType::e_ta);
                    m_timer.stop();
                    // if a schedule cannot be found then continue allocating
                    if(package.finalNode->getData().getScheduleTime() < 0.0)
                    {
                        m_timer.restart();
                        continue;
                    }

                    nlohmann::json metrics = {
                        {"solved", package.foundGoal},
                        {"makespan", package.foundGoal ? package.finalNode->getData().getScheduleTime() : -1},
                        {"nodes_expanded", m_search->nodesExpanded},
                        {"nodes_visited", m_search->nodesSearched},
                        {"Timer", m_timer}};

                    return metrics;
                }

            } while(!m_search->empty());

            nlohmann::json metrics = {{"solved", false},
                                      {"makespan", -1},
                                      {"nodes_expanded", m_search->nodesExpanded},
                                      {"nodes_visited", m_search->nodesSearched},
                                      {"Timer", m_timer}};
            return metrics;
        }

        nlohmann::json Experiments::errorData()
        {
            nlohmann::json metrics = {{"solved", false},
                                      {"makespan", -1},
                                      {"nodes_expanded", m_search->nodesExpanded},
                                      {"nodes_visited", m_search->nodesSearched},
                                      {"Timer", -1}};
            return metrics;
        }

        void Experiments::clear()
        {
            m_search.reset();
        }

        IcraProblem Experiments::getProblem(const unsigned int problem_number)
        {
            const std::string problems_folder = "problems";

            // todo: create config for generating a problem
            nlohmann::json config{
                {"mp",
                 {{"boundary_min", 0.0}, {"boundary_max", 1.0}, {"query_time", 0.001}, {"connection_range", 0.1}}}};

            // Read the maps
            std::vector<std::vector<std::vector<b2PolygonShape>>> maps = readMaps(5);

            const std::string file = fmt::format("{}/problem_{}.json", problems_folder, problem_number);
            IcraProblem problem;
            if(!std::experimental::filesystem::exists(file))
            {
                // Rotate which map to use
                config["mp"]["obstacles"] = maps[problem_number % 5];
                boost::shared_ptr<std::vector<int>> numSpec;
                do
                {
                    problem = IcraProblem::generate(config);
                    numSpec = boost::make_shared<std::vector<int>>(problem.robotTraits()->size(), 1);
                } while(!isAllocatable(
                    problem.goalDistribution(), problem.robotTraits().get(), problem.noncumTraitCutoff(), numSpec));

                // Save problem to file for reuse
                std::ofstream out(file);
                nlohmann::json p_json = problem;
                out << p_json;
                out.flush();
            }
            else
            {
                std::ifstream input(file);
                std::string content((std::istreambuf_iterator<char>(input)), (std::istreambuf_iterator<char>()));
                const nlohmann::json j = nlohmann::json::parse(content);
                problem.init(j);
            }

            return problem;
        }

        std::vector<std::vector<std::vector<b2PolygonShape>>> Experiments::readMaps(const unsigned int nr)
        {
            std::vector<std::vector<std::vector<b2PolygonShape>>> maps;
            for(unsigned int i = 0; i < nr; ++i)
            {
                std::ifstream in(fmt::format("maps/map{}.json", i + 1));
                std::string content((std::istreambuf_iterator<char>(in)), (std::istreambuf_iterator<char>()));
                const nlohmann::json j                       = nlohmann::json::parse(content);
                std::vector<std::vector<b2PolygonShape>> map = j;
                maps.push_back(map);
            }
            return maps;
        }

        void Experiments::handleSignal(int signal)
        {
            Logger::debug("Caught signal {0}", signal);

            auto& experiments = Experiments::getInstance();

            nlohmann::json metrics = {{"solved", false},
                                      {"makespan", -1},
                                      {"nodes_expanded", experiments.m_search->nodesExpanded},
                                      {"nodes_visited", experiments.m_search->nodesSearched},
                                      {"Timer", -1}};

            std::ofstream out(fmt::format("outputs/output_{0:d}_{1:0.2f}_{2:d}.json",
                                          experiments.m_problem_number,
                                          experiments.m_alpha,
                                          experiments.m_sequential ? 1 : 0));
            out << metrics;
            out.flush();

            exit(signal);
        }
    }  // namespace icra2021
}  // namespace grstaps