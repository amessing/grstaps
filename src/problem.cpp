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

// External
#include <earcut.hpp>
#include <fmt/format.h>

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

    void Problem::init(const char* domain_file, const char* problem_file, const char* parameters_file, const char* map_file) {
        PlannerParameters parameters;
        parameters.domainFileName  = domain_file;
        parameters.problemFileName = problem_file;
        {
            parameters.outputFileName    = "problems/0/0/output";
            parameters.generateMutexFile = true;
            parameters.generateTrace = true;
            parameters.generateGroundedDomain = true;
        }
        m_task = Setup::doPreprocess(&parameters);

        std::ifstream ifs(parameters_file);
        nlohmann::json config;
        ifs >> config;

        std::ifstream ifs2(map_file);
        nlohmann::json map;
        ifs2 >> map;

        // TODO: convert map to b2PolygonShape
        std::vector<b2PolygonShape> map_internals = convertBuildingsAndStreetsToPolygons(map["buildings"], map["roads"]);

        mp_min = std::numeric_limits<float>::max();
        mp_max = std::numeric_limits<float>::min();
        {
            b2AABB* box = new b2AABB();
            b2Transform transform;
            transform.SetIdentity();
            for(const b2PolygonShape& poly: map_internals)
            {
                poly.ComputeAABB(box, transform, 0);
                if(box->lowerBound.x < mp_min)
                {
                    mp_min = box->lowerBound.x;
                }
                if(box->lowerBound.y < mp_min)
                {
                    mp_min = box->lowerBound.y;
                }
                if(box->upperBound.x > mp_max)
                {
                    mp_max = box->upperBound.x;
                }
                if(box->upperBound.y > mp_max)
                {
                    mp_max = box->upperBound.y;
                }
            }
            mp_max += 10;
            mp_min -= 10;
            delete box;
        }

        // TODO: parse locations
        for(const nlohmann::json& j: config["streets"])
        {
            m_locations.emplace_back(j["name"].get<std::string>(), j["coord"]["x"].get<float>(), j["coord"]["y"].get<float>());
        }
        for(const nlohmann::json& j: config["buildings"])
        {
            m_locations.emplace_back(j["name"], j["coord"]["x"].get<float>(), j["coord"]["y"].get<float>());
        }
        m_locations.emplace_back("hospital", config["hospital"]["coord"]["x"].get<float>(), config["hospital"]["coord"]["y"].get<float>());
        m_locations.emplace_back("fire_station", config["fire_station"]["coord"]["x"].get<float>(), config["fire_station"]["coord"]["y"].get<float>());
        m_locations.emplace_back("construction_company", config["construction_company"]["coord"]["x"].get<float>(), config["construction_company"]["coord"]["y"].get<float>());

        // m_locations = config["locations"].get<std::vector<Location>>();
        m_starting_locations = config["robot_start_locations"].get<std::vector<unsigned int>>();
        m_map                = std::vector<std::vector<b2PolygonShape>>{
            map_internals,
            {}
        };

        m_robot_traits = config["robot_traits"].get<std::vector<TraitVector>>();
        speedIndex = config["speed_index"];
        mpIndex = config["mp_index"];
        config["mp_boundary_min"] = mp_min;
        config["mp_boundary_max"] = mp_max;
        config["mp_query_time"]= 1.0;
        config["mp_connection_range"] = 0.1;
        m_config = config;
        setWorstMP();

        std::vector<std::string> grounded_action_names = config["grounded_actions"];
        std::vector<std::pair<unsigned int, unsigned int>> action_location = config["actions_start_end"];
        for(int i = 0; i < grounded_action_names.size(); ++i)
        {
            actionToRequirements[grounded_action_names[i]] = i;
            m_action_location_map[grounded_action_names[i]] = action_location[i];
            actionNonCumRequirements.push_back({0.0, 0.0, 0.0, 0.0, 0.0});
        }
        actionRequirements = config["actions_trait_requirements"].get<std::vector<std::vector<float>>>();
    }

    void Problem::setWorstMP()
    {
        longestPath = 0;
        for(int i = 0; i < m_map.size(); i++)
        {
            float currentPath = 0;
            for(int j = 0; j < m_map[i].size(); j++)
            {
                //std::cout << "New" << std::endl;
                for(int k = 0; k < (m_map[i][j]).m_count; k++)
                {
                    if(k == ((m_map[i][j]).m_count - 1))
                    {
                        float x_squared =
                            pow((m_map[i][j]).m_vertices[k].x - ((m_map[i][j]).m_vertices[0].x), 2);
                        float y_squared =
                            pow((m_map[i][j]).m_vertices[k].y - ((m_map[i][j]).m_vertices[0].y), 2);
                        currentPath += sqrt(x_squared + y_squared);
                    }
                    else
                    {
                        float x_squared =
                            pow((m_map[i][j]).m_vertices[k].x - ((m_map[i][j]).m_vertices[k + 1].x), 2);
                        float y_squared =
                            pow((m_map[i][j]).m_vertices[k].y - ((m_map[i][j]).m_vertices[k + 1].y), 2);
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
            longestPath += 4*(mp_max - mp_min);
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
        m_map = obstacles;
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
        return m_map;
    }

    const std::vector<unsigned int>& Problem::startingLocations() const
    {
        return m_starting_locations;
    }

    const nlohmann::json& Problem::config() const
    {
        return m_config;
    }

    inline float computeArea(const std::array<b2Vec2, 3>& triangle)
    {
        const b2Vec2& p = triangle[0];
        const b2Vec2& q = triangle[1];
        const b2Vec2& r = triangle[2];
        return (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
    }

    std::vector<b2PolygonShape> Problem::convertBuildingsAndStreetsToPolygons(const nlohmann::json& buildings, const nlohmann::json& streets)
    {
        std::vector<b2PolygonShape> rv;
        std::vector<std::vector<b2Vec2>> shapes = buildings.get<std::vector<std::vector<b2Vec2>>>();

        std::array<b2Vec2, 3> triangle;
        for(const std::vector<b2Vec2>& shape: shapes)
        {
            std::vector<std::vector<std::array<float, 2>>> polygon;
            polygon.push_back(std::vector<std::array<float, 2>>());
            for(const b2Vec2& point: shape)
            {
                polygon.back().push_back({point.x, point.y});
            }
            // tesselate
            std::vector<int> indices = mapbox::earcut<int>(polygon);
            for(int i = 0; i < indices.size(); i += 3)
            {
                // earcut creates a clockwise ordering and box2d wants counter-clockwise
                // so reverse the triangle
                for(int j = 0; j < 3; ++j)
                {
                    triangle[2 - j] = shape[indices[i + j]];
                }
                const float area = computeArea(triangle);
                if(area < 1E-3)
                {
                    continue;
                }
                // Add triangle
                rv.emplace_back();
                rv.back().Set(triangle.data(), 3);
            }
        }


        shapes = streets.get<std::vector<std::vector<b2Vec2>>>();

        for(const std::vector<b2Vec2>& shape: shapes)
        {
            std::vector<std::vector<std::array<float, 2>>> polygon;
            polygon.push_back(std::vector<std::array<float, 2>>());
            for(const b2Vec2& point: shape)
            {
                polygon.back().push_back({point.x, point.y});
            }

            // tesselate
            std::vector<int> indices = mapbox::earcut<int>(polygon);
            for(int i = 0; i < indices.size(); i += 3)
            {
                // earcut creates a clockwise ordering and box2d wants counter-clockwise
                // so reverse the triangle
                for(int j = 0; j < 3; ++j)
                {
                    triangle[2 - j] = shape[indices[i + j]];
                }
                const float area = computeArea(triangle);
                if(area < 1E-3)
                {
                    continue;
                }

                // Add triangle
                rv.emplace_back();
                rv.back().Set(triangle.data(), 3);
            }
        }

        return rv;
    }
    void Problem::writeMap(const std::string& folder)
    {
        nlohmann::json j = m_map[0];

        std::ofstream output;
        output.open(fmt::format("{}/updated_map.json", folder).c_str());
        output << j.dump(4);
    }
}  // namespace grstaps
