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

        Logger::debug("Grounded Actions: {}", problem.task()->actions.size());

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
        auto numSpec     = boost::make_shared<std::vector<int>>(problem.robotTraits().size(), 1);
        auto robotTraits = &problem.robotTraits();

        Timer timer;
        timer.start();
        std::pair<Plan*, TaskAllocation> last_solution = initialSolve(task_planner,
                                                                      timer,
                                                                      problem,
                                                                      taToSched,
                                                                      *robotTraits,
                                                                      numSpec,
                                                                      isGoal,
                                                                      expander,
                                                                      ns_time);
        if(last_solution.first == nullptr)
        {
            Logger::debug("No solution found");
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
            return m_solution;
        }

        if(tp_anytime)
        {
            last_solution = tpAnytime(last_solution,
                                      task_planner,
                                      timer,
                                      problem,
                                      taToSched,
                                      *robotTraits,
                                      numSpec,
                                      isGoal,
                                      expander,
                                      ns_time);
        }
        else
        {
            last_solution = taAnytime(last_solution,
                                      task_planner,
                                      timer,
                                      problem,
                                      taToSched,
                                      *robotTraits,
                                      numSpec,
                                      isGoal,
                                      expander,
                                      ns_time);
        }

        auto pta = last_solution.second;
        nlohmann::json metrics = {
            {"makespan", pta.getScheduleTime()},
            {"total_grounded_actions", problem.task()->actions.size()},
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
        return m_solution;
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

    std::pair<Plan*, TaskAllocation> SolverSequential::initialSolve(TaskPlanner& task_planner,
                                                                    Timer& timer,
                                                                    Problem& problem,
                                                                    taskAllocationToScheduling& taToSched,
                                                                    std::vector<std::vector<float>>& robotTraits,
                                                                    boost::shared_ptr<std::vector<int>> numSpec,
                                                                    boost::shared_ptr<const AllocationIsGoal> isGoal,
                                                                    boost::shared_ptr<const AllocationExpander> expander,
                                                                    float ns_time)
    {
        Plan* plan;
        std::unique_ptr<SearchResultPackager<TaskAllocation>> package  = std::make_unique<SearchResultPackager<TaskAllocation>>();

        while(!task_planner.emptySearchSpace())
        {
            timer.stop();
            if(timer.get() >= ns_time)
            {
                return std::pair<Plan*, TaskAllocation>(nullptr, TaskAllocation());
            }
            timer.start();

            plan = taskPlanPortion(task_planner);

            timer.stop();
            if(timer.get() >= ns_time)
            {
                return std::pair<Plan*, TaskAllocation>(nullptr, TaskAllocation());
            }
            timer.start();

            if(plan)
            {
                auto orderingCon       = boost::make_shared<std::vector<std::vector<int>>>();
                auto durations         = boost::make_shared<std::vector<float>>();
                auto noncumTraitCutoff = boost::make_shared<std::vector<std::vector<float>>>();
                auto goalDistribution  = boost::make_shared<std::vector<std::vector<float>>>();
                auto actionLocations   = boost::make_shared<std::vector<std::pair<unsigned int, unsigned int>>>();

                setupTaskAllocationParameters(
                    plan, problem, orderingCon, durations, noncumTraitCutoff, goalDistribution, actionLocations);

                if(isAllocatable(goalDistribution, &robotTraits, noncumTraitCutoff, numSpec))
                {
                    taToSched.setActionLocations(actionLocations);

                    TaskAllocation ta(false,
                                      goalDistribution,
                                      &robotTraits,
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
                            return std::pair<Plan*, TaskAllocation>(nullptr, TaskAllocation());
                        }
                        timer.start();

                        graphAllocateAndSchedule.search(isGoal, expander, package.get());

                        timer.stop();
                        if(timer.get() >= ns_time)
                        {
                            return std::pair<Plan*, TaskAllocation>(nullptr, TaskAllocation());
                        }
                        timer.start();

                        m_ta_nodes_expanded += graphAllocateAndSchedule.nodesExpanded;
                        m_ta_nodes_visited += graphAllocateAndSchedule.nodesSearched;
                        // solution found
                        if(package->foundGoal && package->finalNode->getData().getScheduleTime() > 0.0)
                        {
                            Logger::debug("Found a solution");
                            return std::pair<Plan*, TaskAllocation>(plan, package->finalNode->getData());
                        }
                    }
                }
            }
        }
        return std::pair<Plan*, TaskAllocation>(nullptr, TaskAllocation());
    }
    std::pair<Plan*, TaskAllocation> SolverSequential::tpAnytime(std::pair<Plan*, TaskAllocation>& last_solution,
                                                                 TaskPlanner& task_planner,
                                                                 Timer& timer,
                                                                 Problem& problem,
                                                                 taskAllocationToScheduling& taToSched,
                                                                 std::vector<std::vector<float>>& robotTraits,
                                                                 boost::shared_ptr<std::vector<int>> numSpec,
                                                                 boost::shared_ptr<const AllocationIsGoal> isGoal,
                                                                 boost::shared_ptr<const AllocationExpander> expander,
                                                                 float ns_time)
    {
        std::unique_ptr<SearchResultPackager<TaskAllocation>> package  = std::make_unique<SearchResultPackager<TaskAllocation>>();
        Plan* plan;
        while(!task_planner.emptySearchSpace())
        {
            timer.stop();
            if(timer.get() >= ns_time)
            {
                return last_solution;
            }
            timer.start();

            plan = taskPlanPortion(task_planner);

            timer.stop();
            if(timer.get() >= ns_time)
            {
                return last_solution;
            }
            timer.start();

            if(plan)
            {
                auto orderingCon       = boost::make_shared<std::vector<std::vector<int>>>();
                auto durations         = boost::make_shared<std::vector<float>>();
                auto noncumTraitCutoff = boost::make_shared<std::vector<std::vector<float>>>();
                auto goalDistribution  = boost::make_shared<std::vector<std::vector<float>>>();
                auto actionLocations   = boost::make_shared<std::vector<std::pair<unsigned int, unsigned int>>>();

                setupTaskAllocationParameters(
                    plan, problem, orderingCon, durations, noncumTraitCutoff, goalDistribution, actionLocations);

                if(isAllocatable(goalDistribution, &robotTraits, noncumTraitCutoff, numSpec))
                {
                    taToSched.setActionLocations(actionLocations);

                    TaskAllocation ta(false,
                                      goalDistribution,
                                      &robotTraits,
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
                            return last_solution;
                        }
                        timer.start();

                        graphAllocateAndSchedule.search(isGoal, expander, package.get());

                        timer.stop();
                        if(timer.get() >= ns_time)
                        {
                            return last_solution;
                        }
                        timer.start();

                        m_ta_nodes_expanded += graphAllocateAndSchedule.nodesExpanded;
                        m_ta_nodes_visited += graphAllocateAndSchedule.nodesSearched;
                        // solution found
                        if(package->foundGoal && package->finalNode->getData().getScheduleTime() > 0.0)
                        {
                            Logger::debug("Found a solution");
                            if(package->finalNode->getData().getScheduleTime() < last_solution.second.getScheduleTime())
                            {
                                last_solution = std::pair<Plan*, TaskAllocation>(plan, package->finalNode->getData());
                            }
                            break;
                        }
                    }
                }
            }
        }
        return last_solution;
    }


    std::pair<Plan*, TaskAllocation> SolverSequential::taAnytime(std::pair<Plan*, TaskAllocation>& last_solution,
                                                                 TaskPlanner& task_planner,
                                                                 Timer& timer,
                                                                 Problem& problem,
                                                                 taskAllocationToScheduling& taToSched,
                                                                 std::vector<std::vector<float>>& robotTraits,
                                                                 boost::shared_ptr<std::vector<int>> numSpec,
                                                                 boost::shared_ptr<const AllocationIsGoal> isGoal,
                                                                 boost::shared_ptr<const AllocationExpander> expander,
                                                                 float ns_time)
    {
        std::unique_ptr<SearchResultPackager<TaskAllocation>> package  = std::make_unique<SearchResultPackager<TaskAllocation>>();
        Plan* plan = last_solution.first;
        auto orderingCon       = boost::make_shared<std::vector<std::vector<int>>>();
        auto durations         = boost::make_shared<std::vector<float>>();
        auto noncumTraitCutoff = boost::make_shared<std::vector<std::vector<float>>>();
        auto goalDistribution  = boost::make_shared<std::vector<std::vector<float>>>();
        auto actionLocations   = boost::make_shared<std::vector<std::pair<unsigned int, unsigned int>>>();

        setupTaskAllocationParameters(
            plan, problem, orderingCon, durations, noncumTraitCutoff, goalDistribution, actionLocations);

        if(isAllocatable(goalDistribution, &robotTraits, noncumTraitCutoff, numSpec))
        {
            taToSched.setActionLocations(actionLocations);

            TaskAllocation ta(false,
                              goalDistribution,
                              &robotTraits,
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
                    return last_solution;
                }
                timer.start();

                graphAllocateAndSchedule.search(isGoal, expander, package.get());

                timer.stop();
                if(timer.get() >= ns_time)
                {
                    return last_solution;
                }
                timer.start();

                m_ta_nodes_expanded += graphAllocateAndSchedule.nodesExpanded;
                m_ta_nodes_visited += graphAllocateAndSchedule.nodesSearched;
                // solution found
                if(package->foundGoal && package->finalNode->getData().getScheduleTime() > 0.0)
                {
                    Logger::debug("Found a solution");
                    if(package->finalNode->getData().getScheduleTime() < last_solution.second.getScheduleTime())
                    {
                        last_solution = std::pair<Plan*, TaskAllocation>(plan, package->finalNode->getData());
                    }
                }
            }
        }

        return last_solution;
    }
}