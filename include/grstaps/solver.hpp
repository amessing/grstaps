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

#ifndef GRSTAPS_SOLVER_HPP
#define GRSTAPS_SOLVER_HPP

// global
#include <memory>

// external
#include "nlohmann/json.hpp"

namespace grstaps
{
    class MotionPlanner;
    class Problem;
    class Scheduler;
    class Solution;
    class TaskAllocation;
    class TaskPlanner;

    /**
     * Wrapper for all the modules used to solve the problem
     */
    class Solver
    {
    public:
        /**
         * Constructor
         *
         * \param problem The problem to solve
         */
        Solver(std::shared_ptr<Problem> problem,
            std::shared_ptr<MotionPlanner> motion_planner,
            std::shared_ptr<Scheduler> scheduler,
            std::shared_ptr<TaskAllocation> task_allocator,
            std::shared_ptr<TaskPlanner> task_planner);

        /**
         * Runs the system
         *
         * \param config Configuration for how the solver should solve the problem (for comparison tests)
         *
         * \returns The solution if one can be found
         */
        std::shared_ptr<Solution> solve(const nlohmann::json& config);

    private:
        std::shared_ptr<Problem> m_problem;
        std::shared_ptr<MotionPlanner> m_motion_planner;
        std::shared_ptr<Scheduler> m_scheduler;
        std::shared_ptr<TaskAllocation> m_task_allocation;
        std::shared_ptr<TaskPlanner> m_task_planner;
    };
}

#endif //GRSTAPS_SOLVER_HPP
