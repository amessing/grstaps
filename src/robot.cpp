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
#include "grstaps/robot.hpp"

namespace grstaps
{
    unsigned int Robot::s_next_robot_id = 0;

    Robot::Robot(const std::string& species_name)
        : m_id(s_next_robot_id)
        , m_species_name(species_name)
    {}

    Robot::Robot(const std::string& species_name, const std::vector<float>& traits)
        : m_id(s_next_robot_id)
        , m_species_name(species_name)
        , m_traits(traits)
    {}

    unsigned int Robot::id() const
    {
        return m_id;
    }

    const std::string& Robot::speciesName() const
    {
        return m_species_name;
    }

    std::vector<float>& Robot::traits()
    {
        return m_traits;
    }

    const std::vector<float>& Robot::traits() const
    {
        return m_traits;
    }

}  // namespace grstaps