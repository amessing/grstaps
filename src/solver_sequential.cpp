#include "grstaps/solver_sequential.hpp"

#include <boost/make_shared.hpp>
#include <nlohmann/json.hpp>

// local
#include "grstaps/Connections/taskAllocationToScheduling.h"
#include "grstaps/Graph/Graph.h"
#include "grstaps/Graph/Node.h"
#include "grstaps/Scheduling/TAScheduleTime.h"
#include "grstaps/Search/AStarSearch.h"
#include "grstaps/Search/UniformCostSearch.h"
#include "grstaps/Task_Allocation/AllocationDistance.h"
#include "grstaps/Task_Allocation/AllocationExpander.h"
#include "grstaps/Task_Allocation/AllocationIsGoal.h"
#include "grstaps/Task_Allocation/AllocationResultsPackager.h"
#include "grstaps/Task_Allocation/TaskAllocation.h"
#include "grstaps/Task_Allocation/checkAllocatable.h"
#include "grstaps/logger.hpp"
#include "grstaps/motion_planning/motion_planner.hpp"
#include "grstaps/problem.hpp"
#include "grstaps/solution.hpp"
#include "grstaps/task_planning/plan.hpp"
#include "grstaps/task_planning/task_planner.hpp"

namespace grstaps
{
    std::shared_ptr<Solution> SolverSequential::solve(Problem& problem, const float ns_time, bool tp_anytime)
    {
        // Initialize everything
        const nlohmann::json& config = problem.config();

        // Task planner
        TaskPlanner task_planner(problem.task());
        m_tp_nodes_expanded = 0;
        m_tp_nodes_visited  = 0;

        // Motion Planning
        auto motion_planners = setupMotionPlanners(problem);

        // Task Allocation
        taskAllocationToScheduling taToSched(motion_planners, &problem.startingLocations(), problem.longestPath);
        bool usingSpecies = false;
        m_ta_nodes_expanded = 0;
        m_ta_nodes_visited  = 0;

        auto heuristic = boost::make_shared<const AllocationDistance>();
        auto path_cost = boost::make_shared<const TAScheduleTime>();
        auto isGoal    = boost::make_shared<const AllocationIsGoal>();
        auto expander = boost::make_shared<const AllocationExpander>(heuristic, path_cost);
        SearchResultPackager<TaskAllocation>* package            = new AllocationResultsPackager();
        auto numSpec     = boost::make_shared<std::vector<int>>(problem.robotTraits().size(), 1);
        auto robotTraits = &problem.robotTraits();

        Timer timer;
        timer.start();
        Plan* plan;
        std::pair<Plan*, TaskAllocation> last_solution = std::make_pair(nullptr, TaskAllocation());
        while(!task_planner.emptySearchSpace())
        {
            timer.stop();
            if(timer.get() >= ns_time)
            {
                break;
            }
            timer.start();

            plan = taskPlanPortion(task_planner);

            timer.stop();
            if(timer.get() >= ns_time)
            {
                break;
            }
            timer.start();

            if(plan)
            {
                //Timer ta_timer;
                //ta_timer.start();
                auto orderingCon       = boost::make_shared<std::vector<std::vector<int>>>();
                auto durations         = boost::make_shared<std::vector<float>>();
                auto noncumTraitCutoff = boost::make_shared<std::vector<std::vector<float>>>();
                auto goalDistribution  = boost::make_shared<std::vector<std::vector<float>>>();
                auto actionLocations   = boost::make_shared<std::vector<std::pair<unsigned int, unsigned int>>>();

                setupTaskAllocationParameters(
                    plan, problem, orderingCon, durations, noncumTraitCutoff, goalDistribution, actionLocations);

                if(isAllocatable(goalDistribution, robotTraits, noncumTraitCutoff, numSpec))
                {
                    taToSched.setActionLocations(actionLocations);

                    TaskAllocation ta(usingSpecies,
                                      goalDistribution,
                                      robotTraits,
                                      noncumTraitCutoff,
                                      taToSched,
                                      durations,
                                      orderingCon,
                                      numSpec,
                                      problem.speedIndex,
                                      problem.mpIndex);

                    auto root = boost::make_shared<Node<TaskAllocation>>(ta.getID(), ta);
                    root->setData(ta);
                    Graph<TaskAllocation> allocationGraph;
                    allocationGraph.addNode(root);

                    AStarSearch<TaskAllocation> graphAllocateAndSchedule(allocationGraph, root);

                    while(!graphAllocateAndSchedule.empty())
                    {
                        timer.stop();
                        if(timer.get() >= ns_time)
                        {
                            break;
                        }
                        timer.start();

                        graphAllocateAndSchedule.search(isGoal, expander, package);

                        timer.stop();
                        if(timer.get() >= ns_time)
                        {
                            break;
                        }
                        timer.start();

                        m_ta_nodes_expanded += graphAllocateAndSchedule.nodesExpanded;
                        m_ta_nodes_visited += graphAllocateAndSchedule.nodesSearched;
                        // solution found
                        if(package->foundGoal && package->finalNode->getData().getScheduleTime() > 0.0)
                        {
                            // If first solution or a better solution
                            if(!last_solution.first ||
                                package->finalNode->getData().getScheduleTime() < last_solution.second.getScheduleTime())
                            {
                                last_solution.first  = plan;
                                last_solution.second = package->finalNode->getData();
                            }


                            if(tp_anytime)
                            {
                                break;
                            }
                            else // keep going
                            {
                                continue;
                            }
                        }
                    }
                    timer.stop();
                    // Go up to the TP level
                    if(graphAllocateAndSchedule.empty())
                    {
                        timer.start();
                        continue;
                    }
                    // Must be tp_anytime
                    if(timer.get() < ns_time)
                    {
                        timer.start();
                        continue;
                    }
                    auto pta = last_solution.second;
                    nlohmann::json metrics = {
                        {"makespan", pta.getScheduleTime()},
                        {"num_actions", (*pta.actionDurations).size()},
                        {"num_tp_nodes_expanded", m_tp_nodes_expanded},
                        {"num_tp_nodes_visited", m_tp_nodes_visited},
                        {"num_tp_nodes_pruned", 0},
                        {"num_ta_nodes_expanded", m_ta_nodes_expanded},
                        {"num_ta_nodes_visited", m_ta_nodes_visited},
                        {"timer", timer.get()},
                        };
                    auto m_solution = std::make_shared<Solution>(
                        std::shared_ptr<Plan>(last_solution.first),
                        std::make_shared<TaskAllocation>(pta),
                        metrics);
                    delete package;
                    return m_solution;
                }
                // else continue
            }
            else
            {
                timer.stop();
                nlohmann::json metrics = {
                    {"error", "could not find plan"},
                    {"num_tp_nodes_expanded", m_tp_nodes_expanded},
                    {"num_tp_nodes_visited", m_tp_nodes_visited},
                    {"timer", timer.get()}
                };
                auto m_solution = std::make_shared<Solution>(
                    nullptr,
                    nullptr,
                    metrics);
                delete package;
                return m_solution;
            }
        }
        return nullptr;
    }

    Plan* SolverSequential::taskPlanPortion(TaskPlanner& task_planner)
    {
        Plan* base;
        while(!task_planner.emptySearchSpace())
        {
            base = task_planner.poll();
            if(base->isSolution())
            {
                return base;
            }

            ++m_tp_nodes_expanded;
            Logger::debug("Expanding plan: {}", base->id);
            std::vector<Plan*> successors = task_planner.getNextSuccessors(base);
            for(Plan* plan : successors)
            {
                m_tp_nodes_visited += successors.size();
                task_planner.update(base, successors);
            }
        }

        return nullptr;
    }
}