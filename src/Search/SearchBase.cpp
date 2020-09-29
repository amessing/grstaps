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
#ifndef SEARCH_BASE_CPP
#define SEARCH_BASE_CPP

#include <iostream>
#include <stack>

#include "grstaps/Scheduling/TAScheduleTime.h"
#include "grstaps/Search/SearchBase.h"
#include "grstaps/Task_Allocation/TAGoalDist.h"

namespace grstaps
{
    template <class Data>
    SearchBase<Data>::SearchBase(Graph<Data>& g, nodePtr<Data>& initPtr)
        : graph(g)
        , initialNodePtr(initPtr)
    {
        initialNodePtr->setPathCost(0.0f);
    }

    template <class Data>
    SearchBase<Data>::SearchBase()
    {
        graph          = Graph<Data>();
        initialNodePtr = nullptr;
    }
}  // namespace grstaps
#endif