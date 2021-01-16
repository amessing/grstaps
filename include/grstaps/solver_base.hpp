/*
 * Copyright (C) 2021 Andrew Messing
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
#ifndef GRSTAPS_SOLVER_BASE_HPP
#define GRSTAPS_SOLVER_BASE_HPP

// global
#include <memory>
#include <vector>

// external
#include <boost/shared_ptr.hpp>

namespace grstaps
{
    class MotionPlanner;
    class Plan;
    class Problem;
    class Solution;

    class SolverBase
    {
        public:

        /**
         * Runs the solver
         * 
         * \returns the solution if one can be found
         */
        virtual std::shared_ptr<Solution> solve(Problem& problem) = 0;

       protected:

        /**
         * \brief Split up a plan and its ancestry into \p plan_subcomponents
         *
         * @param base
         * @param plan_subcomponents
         */
        void planSubcomponents(const Plan* base, std::vector<const Plan*>& plan_subcomponents);

        boost::shared_ptr<std::vector<boost::shared_ptr<MotionPlanner>>> setupMotionPlanners(const Problem& problem);

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
                                           boost::shared_ptr<std::vector<std::vector<int>>> ordering_constraints,
                                           boost::shared_ptr<std::vector<float>> durations,
                                           boost::shared_ptr<std::vector<std::vector<float>>> noncum_trait_cutoff,
                                           boost::shared_ptr<std::vector<std::vector<float>>> goal_distribution,
                                           boost::shared_ptr<std::vector<std::pair<unsigned int, unsigned int>>> action_locations);

    };
}

#endif GRSTAPS_SOLVER_BASE_HPP