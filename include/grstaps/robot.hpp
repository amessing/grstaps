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
#ifndef GRSTAPS_BASE_ROBOT_HPP
#define GRSTAPS_BASE_ROBOT_HPP

// global
#include <string>

// local
#include "grstaps/noncopyable.hpp"

namespace grstaps
{
    /**
     * Container for a robot/controllable agent
     */
    class Robot : public Noncopyable
    {
       public:
        /**
         * Constructor
         *
         * \param species_name The name of the species of this robot
         */
        Robot(const std::string& species_name);

        /**
         * \returns The identifier for this robot
         */
        unsigned int id() const;

        /**
         * \returns The traits of this robot
         */

       private:
        unsigned int m_id;           //!< The identifier for this specific robot
        std::string m_species_name;  //!< The name of the species of this robot
        // Traits (Vector/Map/Class)
        // Starting location

        static unsigned int s_next_robot_id;  //!< The identifier for the next robot to be created
    };
}  // namespace grstaps

#endif  // GRSTAPS_BASE_ROBOT_HPP
