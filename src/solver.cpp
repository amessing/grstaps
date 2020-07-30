/*
 * Copyright (C) 2020 Andrew Messing
 *
 * GRSTAPS is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 3 of the License,
 * or any later version.
 *
 * GRSTAPS is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GRSTAPS; if not, write to the Free Software Foundation,
 * Inc., #59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "grstaps/solver.hpp"

// local
#include "grstaps/motion_planning/motion_planner.hpp"
#include "grstaps/problem.hpp"
#include "grstaps/solution.hpp"
#include "grstaps/task_planning/plan.hpp"
#include "grstaps/task_planning/task_planner.hpp"
#include <grstaps/Graph/Graph.h>
#include <grstaps/Graph/Node.h>
#include <grstaps/Scheduling/TAScheduleTime.h>
#include <grstaps/Search/AStarSearch.h>
#include <grstaps/Search/UniformCostSearch.h>
#include <grstaps/Task_Allocation/AllocationExpander.h>
#include <grstaps/Task_Allocation/AllocationIsGoal.h>
#include <grstaps/Task_Allocation/AllocationResultsPackager.h>
#include <grstaps/Task_Allocation/TAGoalDist.h>
#include <grstaps/Task_Allocation/TaskAllocation.h>
#include <grstaps/Task_Allocation/checkAllocatable.h>

namespace grstaps
{
    std::shared_ptr<Solution> Solver::solve(Problem& problem)
    {
        // Initialize everything
        const nlohmann::json& config = problem.config();

        // Task planner
        TaskPlanner task_planner(problem.task());
        Plan* base;
        std::vector<Plan*> successors;
        Plan* solution;

        // Motion Planning
        MotionPlanner& motion_planner = MotionPlanner::instance();
        const float boundary_min      = config["mp_boundary_min"];
        const float boundary_max      = config["mp_boundary_max"];
        motion_planner.setMap(problem.obstacles(), boundary_min, boundary_max);

        // Task Allocation
        taskAllocationToScheduling taToSched;
        bool usingSpecies = false;

        // Do these need to be raw pointers?
        // Option 1: leave them
        // Option 2: unique_ptr/shared_ptr
        // Option 3: object and then * when passing them (if they won't be destroyed)

        // Also can any of them be const? That will help with multithreading in the future (fewer mutexes)
        boost::shared_ptr<Heuristic> heur = boost::shared_ptr<Heuristic>(new TAGoalDist());
        boost::shared_ptr<Cost> cos       = boost::shared_ptr<Cost>(new TAScheduleTime());

        boost::shared_ptr<GoalLocator<TaskAllocation>> isGoal =
            boost::shared_ptr<GoalLocator<TaskAllocation>>(new AllocationIsGoal());
        boost::shared_ptr<NodeExpander<TaskAllocation>> expander =
            boost::shared_ptr<NodeExpander<TaskAllocation>>(new AllocationExpander(heur, cos));
        SearchResultPackager<TaskAllocation>* package = new AllocationResultsPackager();

        auto numSpec     = make_shared<vector<int>>(problem.robotTraits().size(), 1);
        auto robotTraits = &problem.robotTraits();

        while(!task_planner.emptySearchSpace())
        {
            base = nullptr;
            successors.clear();
            solution = nullptr;
            task_planner.getNextSuccessors(base, successors, solution);

            int num_children = successors.size();

            std::vector<Plan*> valid_successors;
            std::vector<TaskAllocation*> allocations;

            for(int i = 0; i < num_children; ++i)
            {
                auto orderingCon       = boost::make_shared<std::vector<std::vector<int>>>();
                auto durations         = boost::make_shared<std::vector<float>>();
                auto noncumTraitCutoff = boost::make_shared<std::vector<std::vector<float>>>();
                auto goalDistribution  = boost::make_shared<std::vector<std::vector<float>>>();

                Plan* base = successors[i];
                while(base != nullptr)
                {
                    durations->push_back(base->action->duration[0].exp.value);
                    for(unsigned int j = 0; j < base->orderings.size(); j++)
                    {
                        orderingCon->push_back({firstPoint(base->orderings[j]), secondPoint(base->orderings[j])});
                    }
                    if(base->action->name != "#initial")
                    {
                        noncumTraitCutoff->push_back(problem.actionNonCumRequirements[problem.actionToRequirements[base->action->name]]);
                        goalDistribution->push_back(problem.actionRequirements[problem.actionToRequirements[base->action->name]]);
                    }

                    base = base->parentPlan;
                }

                TaskAllocation ta(usingSpecies,
                                  goalDistribution,
                                  robotTraits,
                                  noncumTraitCutoff,
                                  (&taToSched),
                                  durations,
                                  orderingCon,
                                  numSpec);

                auto node1 = boost::make_shared<Node<TaskAllocation>>(ta.getID(), ta);
                node1->setData(ta);
                Graph<TaskAllocation> allocationGraph;
                allocationGraph.addNode(node1);

                AStarSearch<TaskAllocation> graphAllocateAndSchedule(allocationGraph, node1);

                graphAllocateAndSchedule.search(isGoal, expander, package);
                if(package->foundGoal)
                {
                    successors[i]->h = package->finalNode->getPathCost();
                    valid_successors.push_back(successors[i]);
                }
            }

            // std::copy_if(successors.begin(), successors.end(), std::back_inserter(valid_successors),
            //    [](Plan* p){return p->task_allocatable; });
            // TODO: check if this is correct or backwards
            // std::sort(valid_successors.begin(), valid_successors.end(),
            //    [](Plan* lhs, Plan* rhs){ return lhs->h > rhs->h;});

            for(int i = 0; i < valid_successors.size(); ++i)
            {
                if(valid_successors[i]->isSolution())
                {
                    delete package;
                    auto m_solution = std::make_shared<Solution>(std::shared_ptr<Plan>(successors[i]),
                                                                 std::shared_ptr<TaskAllocation>(allocations[i]));
                    return m_solution;
                }
            }

            task_planner.update(base, valid_successors);
        }

        delete package;
        return nullptr;
    }
}  // namespace grstaps
