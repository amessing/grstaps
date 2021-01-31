#include "grstaps/solver_single_threaded.hpp"

#include <boost/make_shared.hpp>
#include <nlohmann/json.hpp>

// local
#include "grstaps/Connections/taskAllocationToScheduling.h"
#include "grstaps/Graph/Graph.h"
#include "grstaps/Graph/Node.h"
#include "grstaps/Scheduling/TAScheduleTime.h"
#include "grstaps/Search/AStarSearch.h"
#include "grstaps/Search/UniformCostSearch.h"
#include "grstaps/Task_Allocation/AllocationExpander.h"
#include "grstaps/Task_Allocation/AllocationIsGoal.h"
#include "grstaps/Task_Allocation/AllocationResultsPackager.h"
#include "grstaps/Task_Allocation/TAGoalDist.h"
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
    std::shared_ptr<Solution> SolverSingleThreaded::solve(Problem& problem)
    {
        // Initialize everything
        const nlohmann::json& config = problem.config();

        // Task planner
        TaskPlanner task_planner(problem.task());
        unsigned int tplan_nodes_expanded = 0;
        unsigned int tplan_nodes_visited  = 0;
        unsigned int tplan_nodes_pruned   = 0;
        Plan* base;

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

        Timer tp_timer, ta_timer;
        tp_timer.start();
        std::map<Plan*, TaskAllocation> plan_to_ta;

        while(!task_planner.emptySearchSpace())
        {
            base = task_planner.poll();

            if(base->isSolution())
            {
                tp_timer.stop();
                delete package;

                float mp_time = 0;
                for(auto mp: *motion_planners)
                {
                    mp_time += mp->getTotalTime();
                }

                TaskAllocation& ta = plan_to_ta[base];

                nlohmann::json metrics = {
                    {"makespan", ta.getScheduleTime()},
                    {"num_actions", (*ta.actionDurations).size()},
                    {"num_tp_nodes_expanded", tplan_nodes_expanded},
                    {"num_tp_nodes_visited", tplan_nodes_visited},
                    {"num_tp_nodes_pruned", tplan_nodes_pruned},
                    {"num_ta_nodes_expanded", talloc_nodes_expanded},
                    {"num_ta_nodes_visited", talloc_nodes_visited},
                    {"tp_timer", tp_timer.get()},
                    {"ta_timer", ta_timer.get()},
                    {"mp_timer", mp_time}
                };

                auto m_solution =
                    std::make_shared<Solution>(std::shared_ptr<Plan>(base),
                                               std::make_shared<TaskAllocation>(ta),
                                               metrics);

                return m_solution;
            }

            ++tplan_nodes_expanded;
            std::vector<Plan*> successors = task_planner.getNextSuccessors(base);
            unsigned int num_children     = successors.size();
            std::vector<Plan*> valid_successors;
            for(unsigned int i = 0; i < num_children; ++i)
            {
                auto orderingCon       = boost::make_shared<std::vector<std::vector<int>>>();
                auto durations         = boost::make_shared<std::vector<float>>();
                auto noncumTraitCutoff = boost::make_shared<std::vector<std::vector<float>>>();
                auto goalDistribution  = boost::make_shared<std::vector<std::vector<float>>>();
                auto actionLocations   = boost::make_shared<std::vector<std::pair<unsigned int, unsigned int>>>();

                Plan* plan = successors[i];
                setupTaskAllocationParameters(
                    plan, problem, orderingCon, durations, noncumTraitCutoff, goalDistribution, actionLocations);

                taToSched.setActionLocations(actionLocations);
                ta_timer.start();
                if(isAllocatable(goalDistribution, robotTraits, noncumTraitCutoff, numSpec))
                {
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
                    graphAllocateAndSchedule.search(isGoal, expander, package);
                    talloc_nodes_expanded += graphAllocateAndSchedule.nodesExpanded;
                    talloc_nodes_visited += graphAllocateAndSchedule.nodesSearched;
                    ta_timer.stop();
                }
                else
                {
                    ta_timer.stop();
                    continue;
                }

                if(package->foundGoal)
                {
                    //successors[i]->gc = package->finalNode->getData().taToScheduling.sched.getMakeSpan();
                    plan_to_ta[successors[i]] = package->finalNode->getData();
                    valid_successors.push_back(successors[i]);
                }
                else
                {
                    int breakpoint = -1;
                }
            }
            tplan_nodes_visited += valid_successors.size();
            task_planner.update(base, valid_successors);
        }

        tp_timer.stop();
        return nullptr;
    }
}