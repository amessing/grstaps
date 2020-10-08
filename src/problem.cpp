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
#include <iostream>

#include "grstaps/problem.hpp"
#include <iostream>
#include<cmath>

// Local
#include "grstaps/json_conversions.hpp"
#include "grstaps/task_planning/planner_parameters.hpp"
#include "grstaps/task_planning/setup.hpp"


namespace grstaps
{
    Problem::Problem()
        : speedIndex(-1)
        , mpIndex(-1)
    {}

    void Problem::init(char* domain_file, char* problem_file, const char* parameters_file) {
        PlannerParameters parameters;
        parameters.domainFileName  = domain_file;
        parameters.problemFileName = problem_file;
        m_task = Setup::doPreprocess(&parameters);

        std::ifstream ifs(parameters_file);
        nlohmann::json config;
        ifs >> config;

        m_locations = config["locations"].get<std::vector<Location>>();
        m_starting_locations = config["starting_locations"].get<std::vector<unsigned int>>();
        m_obstacles = config["obstacles"].get<std::vector<std::vector<b2PolygonShape>>>();

        m_robot_traits = config["robot_traits"].get<std::vector<TraitVector>>();
        speedIndex = config["speed_index"];
        mpIndex = config["mp_index"];
        m_config = config;

        setWorstMP();
    }

    void Problem::setWorstMP()
    {
        longestPath = 0;
        for(int i = 0; i < m_obstacles.size(); i++)
        {
            float currentPath = 0;
            for(int j = 0; j < m_obstacles[i].size(); j++)
            {
                std::cout << "New" << std::endl;
                for(int k = 0; k < (m_obstacles[i][j]).m_count; k++)
                {
                    if(k == ((m_obstacles[i][j]).m_count - 1))
                    {
                        float x_squared =
                            pow((m_obstacles[i][j]).m_vertices[k].x - ((m_obstacles[i][j]).m_vertices[0].x), 2);
                        float y_squared =
                            pow((m_obstacles[i][j]).m_vertices[k].y - ((m_obstacles[i][j]).m_vertices[0].y), 2);
                        currentPath += sqrt(x_squared + y_squared);
                    }
                    else
                    {
                        float x_squared =
                            pow((m_obstacles[i][j]).m_vertices[k].x - ((m_obstacles[i][j]).m_vertices[k + 1].x), 2);
                        float y_squared =
                            pow((m_obstacles[i][j]).m_vertices[k].y - ((m_obstacles[i][j]).m_vertices[k + 1].y), 2);
                        currentPath += sqrt(x_squared + y_squared);
                    }
                }
            }
            if(currentPath > longestPath)
            {
                longestPath = currentPath;
            }
        }
        if(speedIndex > 0){
            float slowest = std::numeric_limits<float>::max();
            for(int i = 0; i < m_robot_traits.size(); i++){
                if(m_robot_traits[i][speedIndex] < slowest){
                    slowest = m_robot_traits[i][speedIndex];
                }
            }
            longestPath = longestPath/slowest;
        }
    }

    void Problem::configureActions(std::function<void(const std::vector<SASAction>&, Problem*)> configure_function)
    {
        configure_function(m_task->actions, this);
    }

    void Problem::setLocations(const std::vector<Location>& locations)
    {
        m_locations = locations;
    }

    void Problem::setActionLocationMap(
        const std::map<std::string, std::pair<unsigned int, unsigned int>>& action_location_map)
    {
        m_action_location_map = action_location_map;
    }

    void Problem::addActionLocation(const std::string& action, const std::pair<unsigned int, unsigned int>& location)
    {
        m_action_location_map[action] = location;
    }

    void Problem::setActionRequirements(const std::vector<std::vector<float>>& actionReq)
    {
        actionRequirements = actionReq;
    }

    void Problem::setActionNonCumRequirements(const std::vector<std::vector<float>>& actionNonCumReq)
    {
        actionNonCumRequirements = actionNonCumReq;
    }

    void Problem::setStartingLocations(const std::vector<unsigned int>& starting_locations)
    {
        m_starting_locations = starting_locations;
    }

    void Problem::setRobotTraitVector(const std::vector<Problem::TraitVector>& robot_traits)
    {
        m_robot_traits = robot_traits;
    }

    void Problem::setTask(SASTask* task)
    {
        m_task = task;
    }

    void Problem::setObstacles(const std::vector<std::vector<b2PolygonShape>>& obstacles)
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

    const std::pair<unsigned int, unsigned int>& Problem::actionLocation(const std::string& name) const
    {
        if(m_action_location_map.find(name) != m_action_location_map.end())
        {
            return m_action_location_map.at(name);
        }
        throw "Unknown action";
    }

    std::vector<Problem::TraitVector>& Problem::robotTraits()
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

    const std::vector<std::vector<b2PolygonShape>> Problem::obstacles() const
    {
        return m_obstacles;
    }

    const std::vector<unsigned int>& Problem::startingLocations() const
    {
        return m_starting_locations;
    }

    const nlohmann::json& Problem::config() const
    {
        return m_config;
    }
}  // namespace grstaps
