
// Global
#include <experimental/filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <string>
#include <vector>

// External
#include <args.hxx>

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
#include <grstaps/Task_Allocation/checkAllocatable.h>
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

        try
        {
            AStarSearch<TaskAllocation> graphAllocateAndSchedule(allocationGraph, root);
            graphAllocateAndSchedule.search(isGoal, expander, &package);
            time.recordSplit(Timer::SplitType::e_ta);
            time.stop();

            nlohmann::json metrics = {
                {"solved", package.foundGoal},
                {"makespan", package.foundGoal ? package.finalNode->getData().getScheduleTime() : -1},
                {"nodes_expanded", graphAllocateAndSchedule.nodesExpanded},
                {"nodes_visited", graphAllocateAndSchedule.nodesSearched},
                {"timer", time}};

            return metrics;
        }
        catch(std::bad_alloc&)
        {
            time.recordSplit(Timer::SplitType::e_ta);
            time.stop();
            nlohmann::json metrics = {{"solved", false},
                                      {"makespan", -1},
                                      //{"nodes_expanded", graphAllocateAndSchedule.nodesExpanded},
                                      //{"nodes_visited", graphAllocateAndSchedule.nodesSearched},
                                      {"timer", time}};
            return metrics;
        }
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

        try
        {
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
        catch(std::bad_alloc&)
        {
            nlohmann::json metrics = {{"solved", false},
                                      {"makespan", -1},
                                      //{"nodes_expanded", graphAllocateAndSchedule.nodesExpanded},
                                      //{"nodes_visited", graphAllocateAndSchedule.nodesSearched},
                                      {"timer", time}};
            return metrics;
        }
    }

    std::vector<std::vector<std::vector<b2PolygonShape>>> readMaps(const unsigned int nr)
    {
        std::vector<std::vector<std::vector<b2PolygonShape>>> maps;
        for(unsigned int i = 0; i < nr; ++i)
        {
            std::ifstream in(fmt::format("icra_2021_experiments/maps/map{}.json", i + 1));
            std::string content((std::istreambuf_iterator<char>(in)), (std::istreambuf_iterator<char>()));
            const nlohmann::json j                       = nlohmann::json::parse(content);
            std::vector<std::vector<b2PolygonShape>> map = j;
            maps.push_back(map);
        }
        return maps;
    }

    int runV1(int argc, char** argv)
    {
        using IcraProblem = grstaps::icra2021::IcraProblem;

        const unsigned int num_problems = 10;
        const std::string folder        = "icra_2021_experiments/problems";
        if(!std::experimental::filesystem::exists(folder))
        {
            std::experimental::filesystem::create_directories(folder);
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
            if(!std::experimental::filesystem::exists(file))
            {
                // Rotate which map to use
                config["mp"]["obstacles"] = maps[problem_nr % 5];
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
                Logger::debug("solved");
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
                Logger::debug("solved");
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
                Logger::debug("solved");
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
                Logger::debug("solved");
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
                Logger::debug("solved");
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
                Logger::debug("solved");
                round["sequential"] = future.get();
            }

            // solve the problems
            output.push_back(round);
        }

        std::ofstream out("icra2011_results.json");
        out << output;

        return 0;
    }

    int run(bool sequential, int problem_nr, float alpha=0.0)
    {
        using IcraProblem = grstaps::icra2021::IcraProblem;

        const std::string folder = "problems";
        if(!std::filesystem::exists(folder))
        {
            std::filesystem::create_directories(folder);
        }

        const std::string outputs_folder = "outputs";
        if(!std::filesystem::exists(outputs_folder))
        {
            std::filesystem::create_directories(outputs_folder);
        }

        // todo: create config for generating a problem
        nlohmann::json config{
            {"mp", {{"boundary_min", 0.0}, {"boundary_max", 1.0}, {"query_time", 0.001}, {"connection_range", 0.1}}}};

        // Read the maps
        std::vector<std::vector<std::vector<b2PolygonShape>>> maps = readMaps(5);

        const std::string file = folder + fmt::format("/problem_{}.json", problem_nr);
        IcraProblem problem;
        if(!std::filesystem::exists(file))
        {
            // Rotate which map to use
            config["mp"]["obstacles"] = maps[problem_nr % 5];
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
        }
        else
        {
            std::ifstream input(file);
            std::string content((std::istreambuf_iterator<char>(input)), (std::istreambuf_iterator<char>()));
            const nlohmann::json j = nlohmann::json::parse(content);
            problem.init(j);
        }

        Logger::info("Problem {}", problem_nr);

        auto metrics = sequential ? solveSequentially(problem) : solve(problem, alpha);

        std::ofstream out(fmt::format("outputs/output_{}_{}_{}.json", problem_nr, alpha, sequential ? 1 : 0));
        out << metrics;

        return 0;
    }
}  // namespace grstaps

int main(int argc, char** argv)
{
    args::ArgumentParser parser("ICRA Experiments");
    args::ValueFlag<float> alpha(parser, "alpha", "Hyperparameter", {'a'});
    args::ValueFlag<int> problem_nr(parser, "problem", "Problem Number", {'p'});
    args::Flag seq(parser, "sequential", "Flag for running sequential", {'s'});
    try
    {
        parser.ParseCLI(argc, argv);
    }
    catch(args::ParseError e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }
    catch(args::ValidationError e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }

    float a;
    if(alpha)
    {
        a = args::get(alpha);
    }

    int pnr;
    if(problem_nr)
    {
        pnr = args::get(problem_nr);
    }

    if(seq)
    {
        grstaps::run(seq, problem_nr);
    }
    else
    {
        grstaps::run(false, pnr, a);
    }
}