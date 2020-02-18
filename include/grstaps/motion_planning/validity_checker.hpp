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
#ifndef GRSTAPS_VALIDITY_CHECKER_HPP
#define GRSTAPS_VALIDITY_CHECKER_HPP

// global
#include <vector>

// external
#include <box2d/b2_polygon_shape.h>
#include <ompl/base/StateValidityChecker.h>

namespace grstaps
{
    /**
     * Wrapper for using Box2D for collision checking in OMPL
     */
    class ValidityChecker : public ompl::base::StateValidityChecker
    {
       public:
        /**
         * Constructor
         */
        ValidityChecker(const std::vector<b2PolygonShape>& obstacles,
                        const ompl::base::SpaceInformationPtr& space_information);

        /**
         * \return Whether \p state is valid meaning there are no collisions
         */
        virtual bool isValid(const ompl::base::State* state) const override;

       private:
        std::vector<b2PolygonShape> m_obstacles;
    };
}  // namespace grstaps

#endif  // GRSTAPS_VALIDITY_CHECKER_HPP