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

#ifndef GRSTAPS_NODEEXPANDERCPP
#define GRSTAPS_NODEEXPANDERCPP

#include "grstaps/Search/NodeExpander.h"

#include "grstaps/Scheduling/TAScheduleTime.h"
#include "grstaps/Task_Allocation/TAGoalDist.h"

namespace grstaps
{
    /*
    template <class Data>
    NodeExpander<Data>::NodeExpander()
        : heuristicFunc(new TAGoalDist)
        , costFunc(new TAScheduleTime)
    {}
    */

    template <class Data>
    NodeExpander<Data>::NodeExpander(boost::shared_ptr<const Heuristic> heur, boost::shared_ptr<const Cost> cost)
        : heuristicFunc(heur)
        , costFunc(cost)
    {}
}  // namespace grstaps

#endif  // GRSTAPS_NODEEXPANDERCPP
