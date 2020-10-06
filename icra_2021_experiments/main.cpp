
// Global
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

// External
#include <fmt/format.h>

#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <nlohmann/json.hpp>

// GRSTAPS
#include <grstaps/Connections/taskAllocationToScheduling.h>
#include <grstaps/Search/AStarSearch.h>
#include <grstaps/Task_Allocation/TaskAllocation.h>
#include <grstaps/Task_Allocation/TAGoalDist.h>
#include <grstaps/Scheduling/TAScheduleTime.h>
#include <grstaps/Task_Allocation/AllocationIsGoal.h>
#include <grstaps/Task_Allocation/AllocationExpander.h>
#include <grstaps/Task_Allocation/AllocationResultsPackager.h>

// Local
#include "icra_problem.hpp"

namespace grstaps
{
    int runNormal(int argc, char** argv)
    {
        using IcraProblem = grstaps::icra2021::IcraProblem;

        const unsigned int num_problems = 10;
        const std::string folder        = "experiment_configs/problems";
        if(!std::filesystem::exists(folder))
        {
            std::filesystem::create_directories(folder);
        }

        // todo: create config for generating a problem
        nlohmann::json config;

        // Also can any of them be const? That will help with multithreading in the future (fewer mutexes)
        auto heuristic = boost::make_shared<const TAGoalDist>();
        auto path_cost = boost::make_shared<const TAScheduleTime>();

        auto isGoal   = boost::make_shared<const AllocationIsGoal>();
        auto expander = boost::make_shared<const AllocationExpander>(heuristic, path_cost);
        SearchResultPackager<TaskAllocation>* package = new AllocationResultsPackager();


        nlohmann::json output;
        for(unsigned int problem_nr = 0; problem_nr < num_problems; ++problem_nr)
        {
            const std::string file = folder + fmt::format("/problem_{}.json", problem_nr);
            IcraProblem problem;
            if(!std::filesystem::exists(folder))
            {
                // todo: generate problem
                problem = IcraProblem::generate(config);
            }
            else
            {
                // todo: read problem from file
                std::ifstream input(file);
                std::string content((std::istreambuf_iterator<char>(input)), (std::istreambuf_iterator<char>()));
                const nlohmann::json j = content;
                problem.init(j);
            }

            //taskAllocationToScheduling taToSched(problem.motionPlanners(), &problem.startingLocations());


            Timer time;
            time.start();

            TaskAllocation ta(false, // usingSpecies
                              &problem.goalDistribution(),
                              &problem.robotTraits(),
                              &problem.noncumTraitCutoff(),
                              taToSched,
                              &problem.durations(),
                              &problem.orderingConstraints(),
                              numSpec,
                              problem.speedIndex(),
                              problem.mpIndex());

            auto root = boost::make_shared<Node<TaskAllocation>>(ta.getID(), ta);
            root->setData(ta);
            Graph<TaskAllocation> allocationGraph;
            allocationGraph.addNode(root);

            AStarSearch<TaskAllocation> graphAllocateAndSchedule(allocationGraph, root);
            graphAllocateAndSchedule.search(isGoal, expander, package);
            time.recordSplit(Timer::SplitType::e_ta);
            time.stop();

            // todo add makespan
            nlohmann::json metrics = {
                {"nodes_expanded", graphAllocateAndSchedule.nodesExpanded},
                {"nodes_visisted", graphAllocateAndSchedule.nodesSearched},
                {"timer", time}
            };
        }

        return 0;
    }
}

int main(int argc, char** argv)
{
    return grstaps::runNormal(argc, argv);
}