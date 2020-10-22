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

namespace grstaps::icra2021
{
    Experiments& Experiments::getInstance()
    {
        static Experiments instance;
        return instance;
    }

    void Experiments::setProblemVersion(unsigned int version)
    {
        m_problem_version = version;
    }

    nlohmann::json Experiments::solve(const unsigned int problem_number, const float alpha)
    {
        m_problem_number = problem_number;
        m_sequential     = false;
        m_alpha          = alpha;
        Timer::setITAGS();
        std::unique_ptr<IcraProblemBase> problem = getProblem(problem_number);
        Logger::info("Solving with alpha={}", alpha);

        // Also can any of them be const? That will help with multithreading in the future (fewer mutexes)
        auto heuristic = boost::make_shared<const TAGoalDist>(alpha);
        auto path_cost = boost::make_shared<const TAScheduleTime>();

        auto isGoal   = boost::make_shared<const AllocationIsGoal>();
        auto expander = boost::make_shared<const AllocationExpander>(heuristic, path_cost);
        SearchResultPackager<TaskAllocation> package;

        taskAllocationToScheduling taToSched(problem->motionPlanners(), problem->startingLocations().get());
        taToSched.setActionLocations(problem->actionLocations());

        auto numSpec = boost::make_shared<std::vector<int>>(problem->robotTraits()->size(), 1);

        m_timer.start();

        TaskAllocation ta(false,  // usingSpecies
                          problem->goalDistribution(),
                          problem->robotTraits().get(),
                          problem->noncumTraitCutoff(),
                          taToSched,
                          problem->durations(),
                          problem->orderingConstraints(),
                          numSpec,
                          problem->speedIndex(),
                          problem->mpIndex());

        auto root = boost::make_shared<Node<TaskAllocation>>(ta.getID(), ta);
        root->setData(ta);
        Graph<TaskAllocation> allocationGraph;
        allocationGraph.addNode(root);
        m_search = std::make_unique<AStarSearch<TaskAllocation>>(allocationGraph, root);

        m_search->search(isGoal, expander, &package);
        m_timer.recordSplit(Timer::SplitType::e_ta);
        m_timer.stop();

        nlohmann::json metrics = {{"solved", package.foundGoal},
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
        std::unique_ptr<IcraProblemBase> problem = getProblem(problem_number);

        Logger::info("Solving sequentially");
        // Also can any of them be const? That will help with multithreading in the future (fewer mutexes)
        auto heuristic = boost::make_shared<const AllocationDistance>();
        auto path_cost = boost::make_shared<const TAScheduleTime>();

        auto isGoal   = boost::make_shared<const AllocationIsGoal>();
        auto expander = boost::make_shared<const AllocationExpander>(heuristic, path_cost);
        SearchResultPackager<TaskAllocation> package;

        taskAllocationToScheduling taToSched(problem->motionPlanners(), problem->startingLocations().get());
        taToSched.setActionLocations(problem->actionLocations());

        auto numSpec = boost::make_shared<std::vector<int>>(problem->robotTraits()->size(), 1);

        m_timer.start();

        TaskAllocation ta(false,  // usingSpecies
                          problem->goalDistribution(),
                          problem->robotTraits().get(),
                          problem->noncumTraitCutoff(),
                          taToSched,
                          problem->durations(),
                          problem->orderingConstraints(),
                          numSpec,
                          problem->speedIndex(),
                          problem->mpIndex());

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

        return errorData();
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

    std::unique_ptr<IcraProblemBase> Experiments::getProblem(const unsigned int problem_number) const
    {
        const std::string problems_folder = "problems";

        // todo: create config for generating a problem
        nlohmann::json config{
            {"mp", {{"boundary_min", 0.0}, {"boundary_max", 1.0}, {"query_time", 0.001}, {"connection_range", 0.1}}}};

        // Read the maps
        std::vector<std::vector<std::vector<b2PolygonShape>>> maps = readMaps(5);

        const std::string file = fmt::format("{}/problem_{}.json", problems_folder, problem_number);
        std::unique_ptr<IcraProblemBase> problem;
        if(!std::experimental::filesystem::exists(file))
        {
            // Rotate which map to use
            config["mp"]["obstacles"] = maps[problem_number % 5];
            boost::shared_ptr<std::vector<int>> numSpec;
            do
            {
                switch(m_problem_version)
                {
                    case 0:
                        config["num_survivors"] = {
                            {"total", 12}, {"need_food", 12}, {"need_medicine", 0}, {"need_hospital", 0}};
                        config["num_fires"]  = 0;
                        config["num_robots"] = {{"total", 6}, {"ground", 3}, {"aerial", 3}, {"utility", 0}};
                        break;
                    case 1:
                        config["num_survivors"] = {
                            {"total", 12}, {"need_food", 0}, {"need_medicine", 12}, {"need_hospital", 0}};
                        config["num_fires"]  = 0;
                        config["num_robots"] = {{"total", 6}, {"ground", 3}, {"aerial", 3}, {"utility", 0}};
                        break;
                    case 2:
                        config["num_survivors"] = {
                            {"total", 12}, {"need_food", 6}, {"need_medicine", 6}, {"need_hospital", 0}};
                        config["num_fires"]  = 0;
                        config["num_robots"] = {{"total", 6}, {"ground", 3}, {"aerial", 3}, {"utility", 0}};
                        break;
                    case 3:
                        config["num_survivors"] = {
                            {"total", 12}, {"need_food", 4}, {"need_medicine", 4}, {"need_hospital", 4}};
                        config["num_fires"]  = 0;
                        config["num_robots"] = {{"total", 6}, {"ground", 3}, {"aerial", 3}, {"utility", 0}};
                        break;
                    case 4:
                        config["num_survivors"] = {
                            {"total", 12}, {"need_food", 4}, {"need_medicine", 4}, {"need_hospital", 4}};
                        config["num_fires"]  = 0;
                        config["num_robots"] = {{"total", 6}, {"ground", 2}, {"aerial", 4}, {"utility", 0}};
                        break;
                    case 5:
                        config["num_survivors"] = {
                            {"total", 12}, {"need_food", 4}, {"need_medicine", 4}, {"need_hospital", 4}};
                        config["num_fires"]  = 0;
                        config["num_robots"] = {{"total", 6}, {"ground", 4}, {"aerial", 2}, {"utility", 0}};
                        break;
                    case 6:
                        config["num_survivors"] = {
                            {"total", 12}, {"need_food", 4}, {"need_medicine", 4}, {"need_hospital", 4}};
                        config["num_fires"]  = 0;
                        config["num_robots"] = {{"total", 12}, {"ground", 6}, {"aerial", 6}, {"num_utility", 0}};
                        break;
                    case 7:
                        config["num_survivors"] = {

                            {"total", 12}, {"need_food", 4}, {"need_medicine", 4}, {"need_hospital", 4}};
                        config["num_fires"]  = 0;
                        config["num_robots"] = {{"total", 12}, {"ground", 3}, {"aerial", 9}, {"utility", 0}};
                        break;
                    case 8:
                        config["num_survivors"] = {
                            {"total", 12}, {"need_food", 4}, {"need_medicine", 4}, {"need_hospital", 4}};
                        config["num_fires"]  = 0;
                        config["num_robots"] = {{"total", 12}, {"ground", 9}, {"aerial", 3}, {"utility", 0}};
                        break;
                    case 9: {
                        int total = 12;
                        std::random_device rd;
                        std::mt19937 gen(rd());
                        const unsigned int need_food = std::uniform_int_distribution(0, 6)(gen);
                        total -= need_food;
                        const unsigned int need_medicine = std::uniform_int_distribution(0, total)(gen);
                        total -= need_medicine;

                        config["num_survivors"] = {{"total", 12},
                                                   {"need_food", need_food},
                                                   {"need_medicine", need_medicine},
                                                   {"need_hospital", total}};
                        config["num_fires"]     = std::uniform_int_distribution(1, 3)(gen);
                        config["num_robots"]    = {{"total", 12},
                                                {"ground", std::uniform_int_distribution(3, 4)(gen)},
                                                {"aerial", std::uniform_int_distribution(3, 4)(gen)},
                                                {"utility", std::uniform_int_distribution(3, 4)(gen)}};
                    }
                    break;
                    default:
                        throw "help";
                }

                problem = IcraProblemV1::generate(config);
                numSpec = boost::make_shared<std::vector<int>>(problem->robotTraits()->size(), 1);
            } while(!isAllocatable(
                problem->goalDistribution(), problem->robotTraits().get(), problem->noncumTraitCutoff(), numSpec));

            // Save problem to file for reuse
            std::ofstream out(file);
            nlohmann::json p_json = *problem;
            out << p_json;
            out.flush();
        }
        else
        {
            std::ifstream input(file);
            std::string content((std::istreambuf_iterator<char>(input)), (std::istreambuf_iterator<char>()));
            const nlohmann::json j = nlohmann::json::parse(content);
            problem                = std::make_unique<IcraProblemV1>();
            problem->init(j);
        }

        return problem;
    }

    std::vector<std::vector<std::vector<b2PolygonShape>>> Experiments::readMaps(const unsigned int nr)
    {
        std::vector<std::vector<std::vector<b2PolygonShape>>> maps;
        for(unsigned int i = 0; i < nr; ++i)
        {
            std::ifstream in(fmt::format("maps/newmap{}.json", i + 1));
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

        std::ofstream out(fmt::format("outputs/output_{0:d}_{1:0.2f}_{2:d}.json",
                                      experiments.m_problem_number,
                                      experiments.m_alpha,
                                      experiments.m_sequential ? 1 : 0));
        out << experiments.errorData();
        out.flush();

        exit(signal);
    }
}  // namespace grstaps::icra2021