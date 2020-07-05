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


using std::string;
using std::vector;

namespace grstaps
{
    class TaskAllocation;
    class taskAllocationToScheduling{
       public:
        float getNonSpeciesSchedule(TaskAllocation* allocObject);

        float getSpeciesSchedule(TaskAllocation* allocObject);

        float adjustScheduleNonSpeciesSchedule(Scheduler sched, TaskAllocation* TaskAlloc);
       
       private:
        Scheduler sched;
        std::vector<int> concurrent;
        vector<vector<float>> stn;

    };
}

#include <grstaps/Task_Allocation/TaskAllocation.h>
#endif  // GRSTAPS_TASKALLOCATIONTOSCHEDULING_H
