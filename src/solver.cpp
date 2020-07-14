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
//#include "grstaps/motion_planning/motion_planner.hpp"
#include "grstaps/problem.hpp"
#include "grstaps/task_planning/plan.hpp"
#include "grstaps/task_planning/task_planner.hpp"

namespace grstaps
{

    Solver::Solver(std::shared_ptr<Problem> problem,
                   std::shared_ptr<MotionPlanner> motion_planner,
                   std::shared_ptr<Scheduler> scheduler,
                   std::shared_ptr<TaskAllocation> task_allocation,
                   std::shared_ptr<TaskPlanner> task_planner)
        : m_problem(problem)
        , m_motion_planner(motion_planner)
        , m_scheduler(scheduler)
        , m_task_allocation(task_allocation)
        , m_task_planner(task_planner)
    {
    }

    std::shared_ptr<Solution> Solver::solve(const nlohmann::json& config)
    {
        Plan* base;
        std::vector<Plan*> successors;
        Plan* solution;
        while(!m_task_planner->emptySearchSpace())
        {
            base = nullptr;
            successors.clear();
            solution = nullptr;
            m_task_planner->getNextSuccessors(base, successors, solution);

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

            m_task_planner->update(base, valid_successors);
        }

        return nullptr;
    }
}
