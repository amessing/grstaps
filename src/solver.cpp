/*
 * Copyright (C) 2020 Andrew Messing
 *
 * grstaps is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 3 of the License,
 * or any later version.
 *
 * grstaps is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with grstaps; if not, write to the Free Software Foundation,
 * Inc., #59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
/*
#include "grstaps/solver.hpp"

// local
#include "grstaps/motion_planning/motion_planner.hpp"
#include "grstaps/task_planning/task_planner.hpp"

namespace grstaps
{

    Solver::Solver(std::shared_ptr<Problem> problem,
                   std::shared_ptr<MotionPlanner> motion_planner,
                   std::shared_ptr<Scheduler> scheduler,
                   std::shared_ptr<TaskAllocator> task_allocator,
                   std::shared_ptr<TaskPlanner> task_planner)
        : m_problem(problem)
        , m_motion_planner(motion_planner)
        , m_scheduler(scheduler)
        , m_task_allocator(task_allocator)
        , m_task_planner(task_planner)
    {
    }

    std::shared_ptr<Solution> Solver::solve(const nlohmann::json& config)
    {
        // todo: Create base plan

        // todo: loop through search of TP here? passing stuff up and down chain


        return nullptr;
    }
}*/
