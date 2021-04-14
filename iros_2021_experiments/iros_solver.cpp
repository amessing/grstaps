#include "iros_solver.hpp"

#include <fmt/format.h>

#include <boost/make_shared.hpp>
#include <grstaps/Connections/taskAllocationToScheduling.h>
#include <grstaps/Scheduling/TAScheduleTime.h>
#include <grstaps/Task_Allocation/AllocationExpander.h>
#include <grstaps/Task_Allocation/AllocationIsGoal.h>
#include <grstaps/Task_Allocation/AllocationResultsPackager.h>
#include <grstaps/Task_Allocation/TAGoalDist.h>
#include <grstaps/Search/AStarSearch.h>

namespace grstaps
{
    namespace iros2021
    {
        nlohmann::json IrosSolver::solve(IrosProblem &problem)
        {
            // Initialize everything
            const nlohmann::json& config = problem.config();

            // Motion Planning
            auto motion_planners = setupMotionPlanners(problem);

            // Task Allocation
            taskAllocationToScheduling taToSched(motion_planners, &problem.startingLocations(), problem.longestPath);
            bool usingSpecies = false;
            unsigned int talloc_nodes_expanded = 0;
            unsigned int talloc_nodes_visited  = 0;

            // Also can any of them be const? That will help with multithreading in the future (fewer mutexes)
            auto heuristic = boost::make_shared<const TAGoalDist>();
            auto path_cost = boost::make_shared<const TAScheduleTime>();

            auto isGoal    = boost::make_shared<const AllocationIsGoal>();
            auto expander = boost::make_shared<const AllocationExpander>(heuristic, path_cost);
            SearchResultPackager<TaskAllocation>* package            = new AllocationResultsPackager();

            auto numSpec     = boost::make_shared<std::vector<int>>(problem.robotTraits().size(), 1);
            auto robotTraits = &problem.robotTraits();

            Timer timer;

            taToSched.setActionLocations(problem.taskLocations());

            timer.start();
            TaskAllocation ta(false,  // usingSpecies
                              problem.goalDistribution(),
                              &problem.robotTraits(),
                              problem.noncumTraitCutoff(),
                              taToSched,
                              problem.durations(),
                              problem.orderingConstraints(),
                              numSpec,
                              problem.speedIndex,
                              problem.mpIndex);

            auto root = boost::make_shared<Node<TaskAllocation>>(ta.getID(), ta);
            root->setData(ta);
            Graph<TaskAllocation> allocationGraph;
            allocationGraph.addNode(root);
            std::unique_ptr<AStarSearch<TaskAllocation>> search = std::make_unique<AStarSearch<TaskAllocation>>(allocationGraph, root);

            search->search(isGoal, expander, package);
            timer.stop();

            float mp_time = (*motion_planners)[0]->getTotalTime() + (*motion_planners)[1]->getTotalTime();

            nlohmann::json metrics = {{"solved", package->foundGoal},
                                      {"makespan", package->foundGoal ? package->finalNode->getData().getScheduleTime() : -1},
                                      {"nodes_expanded", search->nodesExpanded},
                                      {"nodes_visited", search->nodesSearched},
                                      {"timer", timer.get()},
                                      {"mp_timer", mp_time},
                                      {"solution", solutionToJson(package->finalNode->getData())}
            };

            return metrics;
        }

        boost::shared_ptr<std::vector<boost::shared_ptr<MotionPlanner>>> IrosSolver::setupMotionPlanners(const IrosProblem &problem)
        {
            const std::vector<ClipperLib2::Paths>& maps =  problem.map();
            const nlohmann::json& config = problem.config();

            auto motion_planners = boost::make_shared<std::vector<boost::shared_ptr<MotionPlanner>>>();
            motion_planners->reserve(maps.size());

            const float boundary_min      = config["mp_boundary_min"];
            const float boundary_max      = config["mp_boundary_max"];
            const float query_time        = config["mp_query_time"];
            const float connection_range  = config["mp_connection_range"];

            for(int i = 0; i < maps.size(); ++i)
            {
                auto motion_planner = boost::make_shared<MotionPlanner>();
                motion_planner->setMap(maps[i], boundary_min, boundary_max);
                motion_planner->setLocations(problem.locations());
                motion_planner->setQueryTime(query_time);
                motion_planner->setConnectionRange(connection_range);
                motion_planners->push_back(motion_planner);
            }
            return motion_planners;
        }
        nlohmann::json IrosSolver::solutionToJson(TaskAllocation& allocation)
        {
            nlohmann::json rv;
            const auto motion_plans = allocation.taToScheduling.saveMotionPlanningNonSpeciesSchedule(&allocation);
            rv["allocation"] = allocation.getID();
            rv["motion_plans"] = nlohmann::json();

            unsigned int i = 0;
            for(const auto& agent_motion_plans: motion_plans.second)
            {
                nlohmann::json agent;
                for(const auto& motion_plan: agent_motion_plans)
                {
                    nlohmann::json mp;
                    mp["action_index"] = std::get<0>(motion_plan);
                    mp["start"]     = std::get<1>(motion_plan).first;
                    mp["end"]       = std::get<1>(motion_plan).second;
                    mp["waypoints"] = nlohmann::json();
                    for(const std::pair<float, float>& waypoint: std::get<2>(motion_plan))
                    {
                        nlohmann::json w;
                        w.push_back(waypoint.first);
                        w.push_back(waypoint.second);
                        mp["waypoints"].push_back(w);
                    }
                    agent.push_back(mp);
                }
                rv["motion_plans"][fmt::format("agent_{}", i)] = agent;
                ++i;
            }

            rv["schedule"] = nlohmann::json();
            for(unsigned int i = 0; i < allocation.taToScheduling.sched.actionStartTimes.size(); ++i)
            {
                nlohmann::json action    = {{"index", i},
                                            {"start_time", allocation.taToScheduling.sched.actionStartTimes[i]},
                                            {"end_time", allocation.taToScheduling.sched.stn[i][1]}};
                rv["schedule"].push_back(action);
            }

            return rv;
        }
    }
}