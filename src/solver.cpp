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

namespace grstaps
{
    std::shared_ptr<Solution> Solver::solve(Problem& problem)
    {
        // Initialize everything
         const nlohmann::json& config = problem.config();

         // Task planner
        TaskPlanner task_planner(problem.task());

        // Motion Planning
        MotionPlanner& motion_planner = MotionPlanner::instance();
        const float boundary_min = config["mp_boundary_min"];
        const float boundary_max = config["mp_boundary_max"];
        motion_planner.setMap(problem.obstacles(), boundary_min, boundary_max);

        // Task Allocation

        // Scheduling



        Plan* base;
        std::vector<Plan*> successors;
        Plan* solution;
        while(!task_planner.emptySearchSpace())
        {
            base = nullptr;
            successors.clear();
            solution = nullptr;
            task_planner.getNextSuccessors(base, successors, solution);

            int num_children = successors.size();

            // openmp line
            for(int i = 0; i < num_children; ++i)
            {
                // task allocation & scheduling
                // Compute h and update
            }

            // Filter based on task allocation and sort based on h
            std::vector<Plan*> valid_successors;
            std::copy_if(successors.begin(), successors.end(), std::back_inserter(valid_successors),
                [](Plan* p){return p->task_allocatable; });
            // TODO: check if this is correct or backwards
            std::sort(valid_successors.begin(), valid_successors.end(),
                [](Plan* lhs, Plan* rhs){ return lhs->h > rhs->h;});

            for(int i = 0; i < num_children; ++i)
            {
                if(successors[i]->isSolution())
                {
                    // create and return solution
                }
            }

            task_planner.update(base, valid_successors);
        }

        return nullptr;
    }
}
