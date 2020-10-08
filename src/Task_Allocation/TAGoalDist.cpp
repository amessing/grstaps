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

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <utility>

#include "grstaps/Connections/taskAllocationToScheduling.h"
#include "grstaps/Graph/Node.h"
#include "grstaps/Task_Allocation/TAGoalDist.h"

namespace grstaps
{
    TAGoalDist::TAGoalDist(const float alpha)
        : m_alpha(alpha)
    {}

    float TAGoalDist::operator()(const Graph<TaskAllocation> &graph, const TaskAllocation &parentNode, TaskAllocation &newNode) const
    {
        TAScheduleTime schedule;
        const double makespan = schedule(graph, parentNode, newNode);
        if(makespan == std::numeric_limits<float>::max())
        {
            return makespan;
        }
        const float normalized_schedule_quality = ((makespan - newNode.taToScheduling.sched.bestSchedule) /
                                                   (newNode.taToScheduling.sched.worstSchedule - newNode.taToScheduling.sched.bestSchedule));
        const float percentage_allocated_remaining = newNode.getGoalDistance() / (t toonewNode.startingGoalDistance);
        return m_alpha * normalized_schedule_quality + (1.0 - m_alpha) * percentage_allocated_remaining;
    }

}  // namespace grstaps
