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
#ifndef GRSTAPS_SOLVER_SEQUENTIAL_HPP
#define GRSTAPS_SOLVER_SEQUENTIAL_HPP

#include "grstaps/solver_base.hpp"

namespace grstaps
{
    class TaskPlanner;

    class SolverSequential : public SolverBase
    {
        public:
        /**
         * Runs the solver
         * 
         * \returns the solution if one can be found
         */
        std::shared_ptr<Solution> solve(Problem& problem, const float ns_time, bool tp_anytime);

       protected:
        Plan* taskPlanPortion(TaskPlanner& task_planner);

        std::shared_ptr<Solution> solve(Problem& problem) override { return nullptr; };

        unsigned int m_tp_nodes_expanded;
        unsigned int m_tp_nodes_visited;
        unsigned int m_ta_nodes_expanded;
        unsigned int m_ta_nodes_visited;
    };
}

#endif // GRSTAPS_SOLVER_SEQUENTIAL_HPP