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

#include "grstaps/Task_Allocation/AllocationIsGoal.h"

namespace grstaps
{
    // This operator overloading enables calling
    // operator function () on objects of increment
    bool AllocationIsGoal::operator()(const Graph<TaskAllocation>& graph, nodePtr<TaskAllocation> goalNode) const
    {
        return goalNode->getData().isGoalAllocation();
    }

}  // namespace grstaps