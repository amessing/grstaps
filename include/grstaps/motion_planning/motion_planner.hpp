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
#ifndef GRSTAPS_MOTION_PLANNER_HPP
#define GRSTAPS_MOTION_PLANNER_HPP

// global
#include <utility>

// external
#include <box2d/b2_polygon_shape.h>
#include <ompl/base/Planner.h>
#include <ompl/base/SpaceInformation.h>
#include <ompl/base/spaces/RealVectorStateSpace.h>

// local
#include "grstaps/noncopyable.hpp"

namespace grstaps
{
    // Forward Declarations
    class Location;

    /**
     * Wrapper for Open Motion Planning Library
     *
     * \note Currently using Lazy PRM*
     */
    class MotionPlanner : public Noncopyable
    {
       public:
        /**
         * \returns Singleton to the motion planner
         */
        MotionPlanner& instance();

        /**
         * Sets the obstacles in the map
         *
         * \param obstacles The list of obstacles in the environment
         *
         * \todo: boundary?
         */
        void setMap(const std::vector<b2PolygonShape>& obstacles);

        /**
         * Sets how long a query can run
         */
        void setQueryTime(float run_time);

        /**
         * \param from The idenifier for the location that a robot is travelling from
         * \param to The identifier for the location that a robot is travelling to
         *
         * \returns Whether a motion plan can be created and the length of the motion plan
         */
        std::pair<bool, float> query(unsigned int from, unsigned int to);

        /**
         * \param from The location that a robot is travelling from
         * \param to The location that a robot is travelling to
         *
         * \returns Whether a motion plan can be created and the length of the motion plan
         */
        std::pair<bool, float> query(const Location& from, const Location& to);

       private:
        /**
         * Constructor
         */
        MotionPlanner();

        bool m_map_set;                    //!< Whether the map has been set for the motion planner
        float m_query_time;                //!< How long a query can run for
        ompl::base::PlannerPtr m_planner;  //!< The OMPL motion planner
        std::shared_ptr<ompl::base::RealVectorStateSpace> m_space;  //!< Outline of the space
        ompl::base::SpaceInformationPtr
            m_space_information;  //!< Information about the space (includes validity checker)
    };
}  // namespace grstaps

#endif  // GRSTAPS_MOTION_PLANNER_HPP