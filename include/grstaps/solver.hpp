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
    class Plan;
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
         * Default Constructor
         */
        Solver() = default;

        /**
         * Runs the system
         *
         * \param problem The problem to solve
         *
         * \returns The solution if one can be found
         */
        std::shared_ptr<Solution> solve(Problem& problem);

        static void writeSolution(const std::string& folder);

       private:
        void planSubcomponents(Plan* base, std::vector<const Plan*>& plan_subcomponents);
        std::string planToPDDL();
        void writeTrace(std::ofstream& f, Plan* p);
    };
}  // namespace grstaps

#endif  // GRSTAPS_SOLVER_HPP
