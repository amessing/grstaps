/*
 * Copyright (C)2020 Andrew Messing
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
#include "grstaps/motion_planning/validity_checker.hpp"

// external
#include <ompl/base/spaces/RealVectorStateSpace.h>

namespace grstaps
{
    namespace ob = ompl::base;

    ValidityChecker::ValidityChecker(const std::vector<b2PolygonShape>& obstacles,
                                     const ob::SpaceInformationPtr& space_information)
        : ob::StateValidityChecker(space_information)
        , m_obstacles(obstacles)
    {}

    bool ValidityChecker::isValid(const ob::State* state) const
    {
        b2Transform transform;
        transform.SetIdentity();

        const auto* state_2d = state->as<ob::RealVectorStateSpace::StateType>();
        b2Vec2 point(state_2d->values[0], state_2d->values[1]);

        for(const b2PolygonShape& obstacle: m_obstacles)
        {
            if(obstacle.TestPoint(transform, point))
            {
                return false;
            }
        }
        return true;
    }
}  // namespace grstaps