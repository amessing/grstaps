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
    class taskAllocationToScheduling{
       public:
        float getNonSpeciesSchedule(vector<short> allocation, vector<float>* actionDurations, vector<vector<int>>* orderingConstraint, boost::shared_ptr<vector<int>> numSpecies);

        float getSpeciesSchedule(vector<short> allocation, vector<float>* actionDurations, vector<vector<int>>* orderingConstraints, boost::shared_ptr<vector<int>> numSpecies);
       
       private:
        Scheduler sched;
    };
}
#endif  // GRSTAPS_TASKALLOCATIONTOSCHEDULING_H
