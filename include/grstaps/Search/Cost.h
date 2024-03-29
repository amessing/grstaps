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

#ifndef GSTAPS_COST_H
#define GSTAPS_COST_H

#include "grstaps/Graph/Graph.h"
#include "grstaps/Task_Allocation/TaskAllocation.h"

namespace grstaps
{
    class Cost
    {
       public:
        /**
         *
         * Returns the cost of a node
         *
         * \param the graph that the node is fronm
         * \param id of the parent node
         * \param the new node to find cost of
         *
         */
        virtual float operator()(const Graph<TaskAllocation>& graph, const TaskAllocation& parentNode, TaskAllocation& newNode) const = 0;
    };
}  // namespace grstaps
#endif  // GSTAPS_COST_H
