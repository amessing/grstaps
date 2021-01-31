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
#include "grstaps/motion_planning/clipper_validity_checker.hpp"

// external
#include <ompl/base/spaces/RealVectorStateSpace.h>

namespace grstaps
{
    namespace ob = ompl::base;

    ClipperValidityChecker::ClipperValidityChecker(const ClipperLib2::Paths& internals,
                                     const ob::SpaceInformationPtr& space_information)
        : ob::StateValidityChecker(space_information)
        , m_internals(internals)
    {}

    bool ClipperValidityChecker::isValid(const ob::State* state) const
    {
        const auto* state_2d = state->as<ob::RealVectorStateSpace::StateType>();
        ClipperLib2::IntPoint point(state_2d->values[0] * 1E6, state_2d->values[1] * 1E6);

        int poly_count_inside = 0;
        for (ClipperLib2::Path poly : m_internals)
        {
            const int is_inside_this_poly =
                ClipperLib2::PointInPolygon(point, poly);
            if (is_inside_this_poly == -1)
            {
                return true;
            }
            poly_count_inside += is_inside_this_poly;
        }
        return (poly_count_inside % 2) == 1;
    }
}  // namespace grstaps