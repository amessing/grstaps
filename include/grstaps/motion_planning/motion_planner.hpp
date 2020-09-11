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
#include <map>
#include <mutex>
#include <utility>

// external
#include <box2d/b2_polygon_shape.h>
#include <ompl/base/Planner.h>
#include <ompl/base/SpaceInformation.h>
#include <ompl/base/StateSpace.h>

// local
#include "grstaps/location.hpp"

namespace grstaps
{
    /**
     * Wrapper for Open Motion Planning Library
     *
     * \note Currently using Lazy PRM*
     *
     * \todo: split up ground/flight? (Internal motion planners with boolean flight flags?)
     */
    class MotionPlanner : public Noncopyable
    {
       public:
        /**
         * \returns Singleton to the motion planner
         */
        static MotionPlanner& instance();

        /**
         * Sets the obstacles in the map
         *
         * \param obstacles The list of obstacles in the environment
         * \param boundary_min The minimum value for the x or y axes
         * \param boundary_max The maximum value for the x or y axes
         */
        void setMap(const std::vector<b2PolygonShape>& obstacles, float boundary_min, float boundary_max);

        /**
         * Sets how long a query can run
         *
         * \param run_time The timeout for each query
         */
        void setQueryTime(float run_time);

        /**
         * Sets the maximum distance for two coordinates to be connected
         */
        void setConnectionRange(float range);

        /**
         * Sets a list of possible locations
         */
        void setLocations(const std::vector<Location>& locations);

        /**
         * \param from The identifier for the location that a robot is travelling from
         * \param to The identifier for the location that a robot is travelling to
         *
         * \returns Whether a motion plan can be created and the length of the motion plan
         */
        std::pair<bool, float> query(unsigned int from, unsigned int to);

        std::tuple<bool, float, std::vector<std::pair<float, float>>> getWaypoints(unsigned int from, unsigned int to);

        std::vector<Location> m_locations;
       private:
        /**
         * Constructor
         */
        MotionPlanner();


        bool waypointQuery(unsigned int from, unsigned int to, ompl::base::ProblemDefinitionPtr problem_def);

        bool m_map_set;                     //!< Whether the map has been set for the motion planner
        float m_query_time;                 //!< How long a query can run for
        ompl::base::PlannerPtr m_planner;   //!< The OMPL motion planner
        ompl::base::StateSpacePtr m_space;  //!< Outline of the space
        ompl::base::SpaceInformationPtr
            m_space_information;  //!< Information about the space (includes validity checker)
        std::mutex m_mutex;

        std::map<std::pair<unsigned int, unsigned int>, std::tuple<bool, float, std::vector<std::pair<float, float>>>> m_memory;
    };
}  // namespace grstaps

#endif  // GRSTAPS_MOTION_PLANNER_HPP
