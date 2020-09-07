/*
 * Copyright (C)2020 Glen Neville
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
#ifndef GRSTAPS_TASKALLOCATIONTOSCHEDULING_H
#define GRSTAPS_TASKALLOCATIONTOSCHEDULING_H

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <grstaps/Scheduling/Scheduler.h>

using std::string;
using std::get;
using std::vector;
using start_end          = std::pair<float, float>;
using waypoints          = std::vector<std::pair<float, float>>;
using single_plan        = std::pair<start_end, waypoints>;
using agent_motion_plans = vector<std::pair<start_end, waypoints>>;

namespace grstaps
{
    class MotionPlanner;
    class TaskAllocation;

    class taskAllocationToScheduling
    {
       public:
        /**
         * Constructor
         *
         * \param the motion planner pointer to use for planning
         *
         */
        taskAllocationToScheduling(MotionPlanner* mPlanner                      = nullptr,
                                   const std::vector<unsigned int>* = nullptr);

        /**
         * Get the schedule for a task allocation that does not use species
         *
         * \param the allocation that needs to be scheduled
         *
         * \return the makespan of the schedule
         *
         */

        float getNonSpeciesSchedule(TaskAllocation* allocObject);

        /**
         * Get the schedule for a task allocation that does use species
         *
         * \param the allocation that needs to be scheduled
         *
         * \return the makespan of the schedule
         *
         */
        // todo finish this
        float getSpeciesSchedule(TaskAllocation* allocObject);

        /**
         * Adjust the schedule to account for non allocated actions
         *
         * \param the schedule that needs to be adjusted
         * \param the allocation that needs to be scheduled
         *
         *
         */
        // todo finish this
        void adjustScheduleNonSpeciesSchedule(TaskAllocation* TaskAlloc);

        /**
         * Adjust the schedule to account for motion planning
         *
         * \param the schedule that needs to be adjusted
         * \param the allocation that needs to be scheduled
         *
         *
         */
        float addMotionPlanningNonSpeciesSchedule(TaskAllocation* TaskAlloc);

        /**
         * Save motion plans of agents
         *
         * \param the schedule that needs to be adjusted
         * \param the allocation that needs to be scheduled
         *
         * \return the vector of locations agents will visit in order
         *
         */
        std::pair<bool, vector<agent_motion_plans>> saveMotionPlanningNonSpeciesSchedule(TaskAllocation* TaskAlloc);

        /**
         * Sets a list of the indices of the start and end locations for the actions
         */
        void setActionLocations(boost::shared_ptr<const std::vector<std::pair<unsigned int, unsigned int>>> action_locations);

        Scheduler sched;

       private:
        std::vector<std::vector<float>> stn;
        std::vector<int> actionOrder;
        std::vector<float> maxTraitTeam;
        std::vector<int> concurrent;

        MotionPlanner* m_motion_planner;
        const std::vector<unsigned int>* m_starting_locations;
        boost::shared_ptr<const std::vector<std::pair<unsigned int, unsigned int>>> m_action_locations;

    };
}  // namespace grstaps

#endif  // GRSTAPS_TASKALLOCATIONTOSCHEDULING_H
