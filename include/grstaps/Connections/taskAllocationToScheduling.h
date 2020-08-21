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


#include <grstaps/Scheduling/Scheduler.h>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include <grstaps/motion_planning/motion_planner.hpp>


using std::string;
using std::vector;

namespace grstaps
{
    class TaskAllocation;
    class taskAllocationToScheduling{
       public:

        /**
        * Constructor
        *
        * \param the motion planner pointer to use for planning
        *
        */
        taskAllocationToScheduling(MotionPlanner* mPlanner= NULL, vector<int>* startingLoc= NULL, vector<int>* actionLoc= NULL);

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
        //todo finish this
        float getSpeciesSchedule(TaskAllocation* allocObject);

        /**
        * Adjust the schedule to account for non allocated actions
        *
         * \param the schedule that needs to be adjusted
        * \param the allocation that needs to be scheduled
         *
        *
        */
        //todo finish this
        void adjustScheduleNonSpeciesSchedule(TaskAllocation* TaskAlloc);

        /**
        * Adjust the schedule to account for motion planning
        *
         * \param the schedule that needs to be adjusted
        * \param the allocation that needs to be scheduled
         *
        *
        */
        //todo finish this
        float addMotionPlanningNonSpeciesSchedule(TaskAllocation* TaskAlloc);

        Scheduler sched;

       private:
        std::vector<int> concurrent;
        vector<vector<float>> stn;
        MotionPlanner* motionPlanner;
        vector<int>* startingLocations;
        vector<int>* actionLocations;
        vector<int> actionOrder;
        vector<float> maxTraitTeam;

    };
}

#include <grstaps/Task_Allocation/TaskAllocation.h>
#endif  // GRSTAPS_TASKALLOCATIONTOSCHEDULING_H
