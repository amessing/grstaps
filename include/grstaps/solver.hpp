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

#include <grstaps/Task_Allocation/AllocationResultsPackager.h>

// external
#include "nlohmann/json.hpp"
#include "timer.h"

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

        /**
         * Runs the system in a sequential fashion
         *
         * \param problem The problem to solve
         *
         * \returns The solution if one can be found
         */
        std::shared_ptr<Solution> solveSequentially(Problem& problem);

        void writeSolution(const std::string& folder, std::shared_ptr<Solution> solution);

       private:
        /**
         * \brief Split up a plan and its ancestry into \p plan_subcomponents
         *
         * @param base
         * @param plan_subcomponents
         */
        void planSubcomponents(const Plan* base, std::vector<const Plan*>& plan_subcomponents);

        /**!
         *
         * @param config
         * @param motion_planners
         */
        std::shared_ptr<std::vector<std::shared_ptr<MotionPlanner>>> setupMotionPlanners(const Problem& problem);

        /**!
         * \brief Fills all the vectors needed by task allocation and scheduling
         *
         * @param plan
         * @param ordering_constraints
         * @param durations
         * @param noncum_trait_cutoff
         * @param goal_distribution
         * @param action_locations
         */
        void setupTaskAllocationParameters(const Plan* plan,
                                           const Problem& problem,
                                           std::shared_ptr<std::vector<std::vector<int>>> ordering_constraints,
                                           std::shared_ptr<std::vector<float>> durations,
                                           std::shared_ptr<std::vector<std::vector<float>>> noncum_trait_cutoff,
                                           std::shared_ptr<std::vector<std::vector<float>>> goal_distribution,
                                           std::shared_ptr<std::vector<std::pair<unsigned int, unsigned int>>> action_locations);

        std::string planToPDDL();
        void writeTrace(std::ofstream& f, Plan* p);
        int total_ta_node_expanded;
    };
}  // namespace grstaps

#endif  // GRSTAPS_SOLVER_HPP
