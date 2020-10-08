
// Global
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <string>
#include <vector>

// External
#include <fmt/format.h>

#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <nlohmann/json.hpp>

// GRSTAPS
#include <grstaps/Connections/taskAllocationToScheduling.h>
#include <grstaps/Scheduling/TAScheduleTime.h>
#include <grstaps/Search/AStarSearch.h>
#include <grstaps/Task_Allocation/AllocationDistance.h>
#include <grstaps/Task_Allocation/AllocationExpander.h>
#include <grstaps/Task_Allocation/AllocationIsGoal.h>
#include <grstaps/Task_Allocation/AllocationResultsPackager.h>
#include <grstaps/Task_Allocation/TAGoalDist.h>
#include <grstaps/Task_Allocation/TaskAllocation.h>
#include <grstaps/json_conversions.hpp>
#include <grstaps/logger.hpp>

// Local
#include "icra_problem.hpp"

namespace grstaps
{
    nlohmann::json solve(grstaps::icra2021::IcraProblem problem, const float alpha)
    {
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

        Timer time;
        time.start();

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

        AStarSearch<TaskAllocation> graphAllocateAndSchedule(allocationGraph, root);
        graphAllocateAndSchedule.search(isGoal, expander, &package);
        time.recordSplit(Timer::SplitType::e_ta);
        time.stop();

        nlohmann::json metrics = {{"solved", package.foundGoal},
                                  {"makespan", package.foundGoal ? package.finalNode->getData().getScheduleTime() : -1},
                                  {"nodes_expanded", graphAllocateAndSchedule.nodesExpanded},
                                  {"nodes_visited", graphAllocateAndSchedule.nodesSearched},
                                  {"timer", time}};

        return metrics;
    }

    nlohmann::json solveSequentially(grstaps::icra2021::IcraProblem problem)
    {
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

        Timer time;
        time.start();

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

        AStarSearch<TaskAllocation> graphAllocateAndSchedule(allocationGraph, root);

        do
        {
            graphAllocateAndSchedule.search(isGoal, expander, &package);
            if(package.foundGoal)
            {
                time.recordSplit(Timer::SplitType::e_ta);
                time.stop();
                // if a schedule cannot be found then continue allocating
                if(package.finalNode->getData().getScheduleTime() < 0.0)
                {
                    time.restart();
                    continue;
                }

                nlohmann::json metrics = {
                    {"solved", package.foundGoal},
                    {"makespan", package.foundGoal ? package.finalNode->getData().getScheduleTime() : -1},
                    {"nodes_expanded", graphAllocateAndSchedule.nodesExpanded},
                    {"nodes_visited", graphAllocateAndSchedule.nodesSearched},
                    {"timer", time}};

                return metrics;
            }

        } while(!graphAllocateAndSchedule.empty());

        nlohmann::json metrics = {{"solved", false},
                                  {"makespan", -1},
                                  {"nodes_expanded", graphAllocateAndSchedule.nodesExpanded},
                                  {"nodes_visited", graphAllocateAndSchedule.nodesSearched},
                                  {"timer", time}};

        return metrics;
    }

    std::vector<std::vector<std::vector<b2PolygonShape>>> readMaps(const unsigned int nr)
    {
        std::vector<std::vector<std::vector<b2PolygonShape>>> maps;
        for(unsigned int i = 0; i < nr; ++i)
        {
            std::ifstream in(fmt::format("experiment_configs/icra_2021_experiments/maps/map{}.json", i + 1));
            std::string content((std::istreambuf_iterator<char>(in)), (std::istreambuf_iterator<char>()));
            const nlohmann::json j                       = nlohmann::json::parse(content);
            std::vector<std::vector<b2PolygonShape>> map = j;
            maps.push_back(map);
        }
        return maps;
    }

    int run(int argc, char** argv)
    {
        using IcraProblem = grstaps::icra2021::IcraProblem;

        const unsigned int num_problems = 10;
        const std::string folder        = "experiment_configs/problems";
        if(!std::filesystem::exists(folder))
        {
            std::filesystem::create_directories(folder);
        }

        // todo: create config for generating a problem
        nlohmann::json config{
            {"mp", {{"boundary_min", 0.0}, {"boundary_max", 1.0}, {"query_time", 0.001}, {"connection_range", 0.1}}}};

        // Read the maps
        std::vector<std::vector<std::vector<b2PolygonShape>>> maps = readMaps(5);

        nlohmann::json output;
        for(unsigned int problem_nr = 0; problem_nr < num_problems; ++problem_nr)
        {
            const std::string file = folder + fmt::format("/problem_{}.json", problem_nr);
            IcraProblem problem;
            if(!std::filesystem::exists(file))
            {
                // Rotate which map to use
                config["mp"]["obstacles"] = maps[problem_nr % 5];
                problem                   = IcraProblem::generate(config);

                // Save problem to file for reuboost::shared_ptr<se
                std::ofstream out(file);
                nlohmann::json p_json = problem;
                out << p_json;
            }
            else
            {
                std::ifstream input(file);
                std::string content((std::istreambuf_iterator<char>(input)), (std::istreambuf_iterator<char>()));
                const nlohmann::json j = nlohmann::json::parse(content);
                problem.init(j);
            }

            Logger::info("Problem {}", problem_nr);

            nlohmann::json round = {{"problem_nr", problem_nr}};

            // 30 minutes
            std::chrono::seconds span(1800);

            // Alpha = 0
            std::future<nlohmann::json> future = std::async(std::launch::async, solve, problem, 0.0f);
            if(future.wait_for(span) == std::future_status::timeout)
            {
                Logger::debug("timedout");
                round["alpha_0"] = {{"solved", false}, {"makespan", -1}, {"timer", 1800.0}};
            }
            else
            {
                round["alpha_0"] = future.get();
            }

            // Alpha = 0.25
            future = std::async(std::launch::async, solve, problem, 0.25);
            if(future.wait_for(span) == std::future_status::timeout)
            {
                Logger::debug("timedout");
                round["alpha_0.25"] = {{"solved", false}, {"makespan", -1}, {"timer", 1800.0}};
            }
            else
            {
                round["alpha_0.25"] = future.get();
            }

            // Alpha = 0.5
            future = std::async(std::launch::async, solve, problem, 0.5);
            if(future.wait_for(span) == std::future_status::timeout)
            {
                Logger::debug("timedout");
                round["alpha_0.5"] = {{"solved", false}, {"makespan", -1}, {"timer", 1800.0}};
            }
            else
            {
                round["alpha_0.5"] = future.get();
            }

            // Alpha = 0.75
            future = std::async(std::launch::async, solve, problem, 0.75);
            if(future.wait_for(span) == std::future_status::timeout)
            {
                Logger::debug("timedout");
                round["alpha_0.75"] = {{"solved", false}, {"makespan", -1}, {"timer", 1800.0}};
            }
            else
            {
                round["alpha_0.75"] = future.get();
            }

            // Alpha = 1.0
            future = std::async(std::launch::async, solve, problem, 1.0f);
            if(future.wait_for(span) == std::future_status::timeout)
            {
                Logger::debug("timedout");
                round["alpha_1.0"] = {{"solved", false}, {"makespan", -1}, {"timer", 1800.0}};
            }
            else
            {
                round["alpha_1.0"] = future.get();
            }

            // Sequential
            future = std::async(std::launch::async, solveSequentially, problem);
            if(future.wait_for(span) == std::future_status::timeout)
            {
                Logger::debug("timedout");
                round["sequential"] = {{"solved", false}, {"makespan", -1}, {"timer", 1800.0}};
            }
            else
            {
                round["sequential"] = future.get();
            }

            // solve the problems
            output.push_back(round);
        }

        std::ofstream out("icra2011_results.json");
        out << output;

        return 0;
    }
}  // namespace grstaps

int main(int argc, char** argv)
{
    return grstaps::run(argc, argv);
}