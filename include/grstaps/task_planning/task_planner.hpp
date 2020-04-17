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
#ifndef GRSTAPS_TASK_PLANNER_HPP
#define GRSTAPS_TASK_PLANNER_HPP

// global
#include <memory>

// external
#include "nlohmann/json.hpp"

// local
#include "grstaps/noncopyable.hpp"
#include "grstaps/task_planning/utils.hpp"

namespace grstaps
{
    // Forward Declarations
    class Plan;
    class Problem;
    class Solution;

    /**
     * Interface for a task planner
     */
    class TaskPlanner : public Noncopyable
    {
    public:
        /**
         * Constructor
         *
         * \param config Any configuration parameters for the planner
         */
        TaskPlanner(const nlohmann::json& config);

        /**
         * Creates the root node
         */
        virtual std::shared_ptr<Plan> createRoot(const StateAssignment& state) = 0;

        /**
         * \returns Whether the search space is empty
         */
        virtual bool emptySearchSpace() const = 0;

        /**
         * Polls for the best plan
         */
        virtual std::shared_ptr<Plan> poll() = 0;

        /**
         * Creates a list of the possible successors of a plan
         *
         * \param base The node to expand
         */
        virtual std::vector<std::shared_ptr<Plan>> possibleSuccessors(std::shared_ptr<Plan> base) = 0;

        /**
         * Adds the valid successors to the tree
         *
         * \param The node that was expanded
         * \param possibles A list of child plans
         * \param valid A list of which child plans are valid
         */
        virtual void addSuccessors(std::shared_ptr<Plan> base, const std::vector<std::shared_ptr<Plan>>& possibles, const std::vector<bool>& valid) = 0;
    protected:
        friend class Solver;
    };
}

#endif //GRSTAPS_TASK_PLANNER_HPP
