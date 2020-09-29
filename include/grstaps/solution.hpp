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
#ifndef GRSTAPS_SOLUTION_HPP
#define GRSTAPS_SOLUTION_HPP

// global
#include <memory>

#include <grstaps/Task_Allocation/TaskAllocation.h>
#include <grstaps/Timer.h>

// Task Allocation
namespace grstaps
{
    // Forward Declarations
    class Plan;

    /**
     * The solution to a MA-CTAMP problem
     */
    class Solution
    {
       public:
        /**
         * Constructor
         *
         * \param plan A partial-order plan
         */
        Solution(std::shared_ptr<Plan> plan,
                 std::shared_ptr<TaskAllocation> allocation,
                 const nlohmann::json& metrics);

        /**
         * Writes the solution to file
         *
         * \param filepath The path to the file which to write the plan
         */
        void write(const std::string& filepath);

        /**
         * \returns The partial-order plan from this solution
         */
        const Plan& plan() const;

        /**
         * \returns The task
         */
        const TaskAllocation& allocation() const;

        /**
         * \returns The metrics
         */
        const nlohmann::json& metrics() const;

       private:
        std::shared_ptr<Plan> m_plan;  //!< A partial-order plan
        std::shared_ptr<grstaps::TaskAllocation> m_allocation;
        nlohmann::json m_metrics;

        void planSubcomponents(Plan* base, std::vector<const Plan*>& plan_subcomponents);
    };
}  // namespace grstaps

#endif  // GRSTAPS_SOLUTION_HPP
