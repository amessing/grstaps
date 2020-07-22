/*
 * Copyright (C) 2020 Andrew Messing
 *
 * grstaps is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 3 of the License,
 * or any later version.
 *
 * grstaps is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with grstaps; if not, write to the Free Software Foundation,
 * Inc., #59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#include "grstaps/problem.hpp"

namespace grstaps
{
    void Problem::setLocations(const std::vector<Location>& locations)
    {
        m_locations = locations;
    }

    void Problem::setRobotTraitVector(const std::vector<Problem::TraitVector>& robot_traits)
    {
        m_robot_traits = robot_traits;
    }

    void Problem::setTask(SASTask* task)
    {
        m_task = task;
    }

    void Problem::setObstacles(const std::vector<b2PolygonShape>& obstacles)
    {
        m_obstacles = obstacles;
    }

    void Problem::setConfig(const nlohmann::json& config)
    {
        m_config = config;
    }

    const std::vector<Location>& Problem::locations() const
    {
        return m_locations;
    }

    const Location& Problem::location(uint i) const
    {
        return m_locations[i];
    }

    const std::vector<Problem::TraitVector>& Problem::robotTraits() const
    {
        return m_robot_traits;
    }

    const Problem::TraitVector& Problem::robotTrait(uint i) const
    {
        return m_robot_traits[i];
    }

    SASTask* Problem::task()
    {
        return m_task;
    }

    const SASTask* Problem::task() const
    {
        return m_task;
    }

    const std::vector<b2PolygonShape>& Problem::obstacles() const
    {
        return m_obstacles;
    }

    const nlohmann::json& Problem::config() const
    {
        return m_config;
    }
}
