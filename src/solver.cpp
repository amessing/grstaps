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
#include "grstaps/task_planning/plan.hpp"
#include "grstaps/task_planning/task_planner.hpp"
#include "grstaps/solution.hpp"
#include <grstaps/Task_Allocation/checkAllocatable.h>
#include <grstaps/Graph/Node.h>
#include <grstaps/Graph/Graph.h>
#include <grstaps/Search/UniformCostSearch.h>
#include <grstaps/Task_Allocation/AllocationExpander.h>
#include <grstaps/Task_Allocation/TaskAllocation.h>
#include <grstaps/Task_Allocation/AllocationIsGoal.h>
#include <grstaps/Task_Allocation/AllocationResultsPackager.h>
#include <grstaps/Scheduling/TAScheduleTime.h>
#include <grstaps/Task_Allocation/TAGoalDist.h>
#include <grstaps/Search/AStarSearch.h>

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
        const float boundary_min = config["mp_boundary_min"];
        const float boundary_max = config["mp_boundary_max"];
        motion_planner.setMap(problem.obstacles(), boundary_min, boundary_max);

        // Task Allocation
        taskAllocationToScheduling taToSched;
        bool usingSpecies = false;
        Heuristic *heur = new TAGoalDist();
        Cost *cos = new TAScheduleTime();
        GoalLocator<TaskAllocation> *isGoal = new AllocationIsGoal();
        NodeExpander<TaskAllocation> *expander = new AllocationExpander(heur, cos);
        SearchResultPackager<TaskAllocation> *package = new AllocationResultsPackager();
        shared_ptr<vector<int>> numSpec =  shared_ptr<vector<int>>(new vector<int>(problem.robotTraits().size(),1));
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

            vector<int> remove;
            for(int i = 0; i < num_children; ++i)
            {

                boost::shared_ptr<vector<vector<int>>> orderingCon;
                boost::shared_ptr<vector<float>> durations;
                shared_ptr<vector<vector<float>>> noncumTraitCutoff;
                boost::shared_ptr<vector<vector<float>>> goalDistribution;

                TaskAllocation ta(usingSpecies, goalDistribution, robotTraits, noncumTraitCutoff, (&taToSched), durations, orderingCon, numSpec);

                auto node1 = boost::shared_ptr<Node<TaskAllocation>>(new Node<TaskAllocation>(std::string(ta.getID()), ta));
                node1->setData(ta);
                Graph<TaskAllocation> allocationGraph;
                allocationGraph.addNode(node1);

                AStarSearch<TaskAllocation> graphAllocateAndSchedule(allocationGraph, node1);
                graphAllocateAndSchedule.search(isGoal, expander, package);
                if(package->foundGoal){
                    successors[i]->h = package->finalNode->getPathCost();
                }
                else{
                    remove.push_back(i);
                }
            }
            for(int i = remove.size(); i > 0; ++i){
                successors.erase(successors.begin() + remove[i-1]);
            }

            std::copy_if(successors.begin(), successors.end(), std::back_inserter(valid_successors),
                [](Plan* p){return p->task_allocatable; });
            // TODO: check if this is correct or backwards
            std::sort(valid_successors.begin(), valid_successors.end(),
                [](Plan* lhs, Plan* rhs){ return lhs->h > rhs->h;});

            for(int i = 0; i < num_children; ++i)
            {
                if(successors[i]->isSolution())
                {
                    delete heur;
                    delete cos;
                    delete isGoal;
                    delete expander;
                    delete package;
                    std::shared_ptr<Solution> m_solution(new Solution(std::shared_ptr<Plan>(successors[i]), std::shared_ptr<TaskAllocation>(allocations[i])));
                    return m_solution;
                }
            }

            task_planner.update(base, valid_successors);
        }

        delete heur;
        delete cos;
        delete isGoal;
        delete expander;
        delete package;
        return nullptr;
    }
}
