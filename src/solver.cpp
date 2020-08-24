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
#include "grstaps/logger.hpp"
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
            base = task_planner.poll();
            Logger::debug("Expanding plan: {}", base->id);
            task_planner.writeTrace(std::cout, base);
            std::vector<Plan*> successors = task_planner.getNextSuccessors(base);
            unsigned int num_children     = successors.size();

            std::vector<std::tuple<Plan*, TaskAllocation*>> potential_successors;

            // openmp line
            for(unsigned int i = 0; i < num_children; ++i)
            {
                auto orderingCon       = boost::make_shared<std::vector<std::vector<int>>>();
                auto durations         = boost::make_shared<std::vector<float>>();
                auto noncumTraitCutoff = boost::make_shared<std::vector<std::vector<float>>>();
                auto goalDistribution  = boost::make_shared<std::vector<std::vector<float>>>();

                Plan* plan = successors[i];

                // Fill in vectors for TA and Scheduling
                std::vector<const Plan*> plan_subcomponents;
                planSubcomponents(plan, plan_subcomponents);
                std::set<std::pair<uint16_t, uint16_t>> order_constraints;
                for(unsigned int j = 0; j < plan_subcomponents.size(); ++j)
                {
                    const Plan* p = plan_subcomponents[j];
                    for(unsigned int k = 0; k < p->orderings.size(); ++k)
                    {
                        // uint16_t
                        TTimePoint fp = firstPoint(p->orderings[k]);
                        TTimePoint sp = secondPoint(p->orderings[k]);
                        // Time points are based on start and end snap actions
                        // Also include the initial action

                        order_constraints.insert({fp / 2 - 1, sp / 2 - 1});
                    }
                    if(j > 0)
                    {
                        durations->push_back(plan->action->duration[0].exp.value);

                        noncumTraitCutoff->push_back(
                            problem.actionNonCumRequirements[problem.actionToRequirements[p->action->name]]);
                        goalDistribution->push_back(
                            problem.actionRequirements[problem.actionToRequirements[p->action->name]]);
                    }
                }
                for(auto oc: order_constraints)
                {
                    orderingCon->push_back({oc.first, oc.second});
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
                if(package->foundGoal || true)
                {
                    successors[i]->h = package->finalNode->getPathCost();
                    potential_successors.push_back({successors[i], &package->finalNode->getData()});
                }
            }

            // std::copy_if(successors.begin(), successors.end(), std::back_inserter(valid_successors),
            //    [](Plan* p){return p->task_allocatable; });

            // TODO: check if this is correct or backwards
            std::sort(potential_successors.begin(),
                      potential_successors.end(),
                      [](std::tuple<Plan*, TaskAllocation*> lhs, std::tuple<Plan*, TaskAllocation*> rhs) {
                          return std::get<0>(lhs)->h > std::get<0>(rhs)->h;
                      });

            for(unsigned int i = 0; i < potential_successors.size(); ++i)
            {
                if(std::get<0>(potential_successors[i])->isSolution())
                {
                    delete package;
                    auto m_solution = std::make_shared<Solution>(
                        std::shared_ptr<Plan>(std::get<0>(potential_successors[i])),
                        std::shared_ptr<TaskAllocation>(std::get<1>(potential_successors[i])));
                    return m_solution;
                }
            }

            std::vector<Plan*> valid_successors;
            for(auto it = potential_successors.begin(), end = potential_successors.end(); it != end; ++it)
            {
                valid_successors.push_back(std::move(std::get<0>(*it)));
            }

            task_planner.update(base, valid_successors);
        }

        delete package;
        return nullptr;
    }

    void Solver::planSubcomponents(Plan* base, std::vector<const Plan*>& plan_subcomponents)
    {
        if(base == nullptr)
        {
            plan_subcomponents.clear();
        }
        else
        {
            planSubcomponents(base->parentPlan, plan_subcomponents);
            plan_subcomponents.push_back(base);
        }
    }
}  // namespace grstaps
