/*
 * Copyright (C) 2021 Andrew Messing
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
#include "iros_problem.hpp"

// Global
#include <fstream>
#include <iostream>
#include <cmath>

// External
#include <fmt/format.h>

// Local
#include <boost/make_shared.hpp>
#include <grstaps/json_conversions.hpp>

namespace grstaps
{
    namespace iros2021
    {
        IrosProblem::IrosProblem()
            : speedIndex(-1)
            , mpIndex(-1)
        {}

        void IrosProblem::init(const char* parameters_file, const char* map_file)
        {
            std::ifstream ifs(parameters_file);
            nlohmann::json config;
            ifs >> config;

            std::ifstream ifs2(map_file);
            nlohmann::json map;
            ifs2 >> map;

            // groundfolder
            m_map.push_back(convertBuildingsAndStreetsToPolygons2(map["buildings"], map["roads"]));

            mp_min = std::numeric_limits<float>::max();
            mp_max = std::numeric_limits<float>::min();
            {
                for(const ClipperLib2::Path& poly: m_map[0])
                {
                    for(const ClipperLib2::IntPoint& point: poly)
                    {
                        mp_min = std::min<float>(static_cast<float>(std::min(point.X, point.Y)), mp_min);
                        mp_max = std::max<float>(static_cast<float>(std::max(point.X, point.Y)), mp_max);
                    }
                }
                mp_max += 10;
                mp_min -= 10;
            }

            // Aerial
            ClipperLib2::Path boundary = {ClipperLib2::IntPoint(mp_min, mp_min),
                                          ClipperLib2::IntPoint(mp_min, mp_max),
                                          ClipperLib2::IntPoint(mp_max, mp_max),
                                          ClipperLib2::IntPoint(mp_max, mp_min)};
            if(ClipperLib2::Area(boundary) < 0)
            {
                std::reverse(boundary.begin(), boundary.end());
            }
            m_map.push_back({boundary});

            for(const nlohmann::json& j: config["streets"])
            {
                m_locations.emplace_back(
                    j["name"].get<std::string>(), j["coord"]["x"].get<float>(), j["coord"]["y"].get<float>());
            }
            for(const nlohmann::json& j: config["buildings"])
            {
                m_locations.emplace_back(j["name"], j["coord"]["x"].get<float>(), j["coord"]["y"].get<float>());
            }
            m_locations.emplace_back("hospital",
                                     config["hospital"]["coord"]["x"].get<float>(),
                                     config["hospital"]["coord"]["y"].get<float>());
            m_locations.emplace_back("fire_station",
                                     config["fire_station"]["coord"]["x"].get<float>(),
                                     config["fire_station"]["coord"]["y"].get<float>());
            m_locations.emplace_back("construction_company",
                                     config["construction_company"]["coord"]["x"].get<float>(),
                                     config["construction_company"]["coord"]["y"].get<float>());

            m_starting_locations = config["robot_start_locations"].get<std::vector<unsigned int>>();

            m_ordering_constraints = boost::make_shared<std::vector<std::vector<int>>>(config["ordering_constraints"].get<std::vector<std::vector<int>>>());
            m_durations = boost::make_shared<std::vector<float>>(config["tasks_durations"].get<std::vector<float>>());
            m_goal_distribution = boost::make_shared<std::vector<std::vector<float>>>(config["tasks_trait_requirements"].get<std::vector<std::vector<float>>>());
            m_noncum_trait_cutoff = boost::make_shared<std::vector<std::vector<float>>>();
            const int num_traits = (*m_goal_distribution)[0].size();
            for(int i = 0; i < m_goal_distribution->size(); ++i)
            {
                m_noncum_trait_cutoff->push_back(std::vector<float>());
                for(int j = 0; j < num_traits; ++j)
                {
                    m_noncum_trait_cutoff->back().push_back(0);
                }
            }

            m_robot_traits                = config["robot_traits"].get<std::vector<TraitVector>>();
            speedIndex                    = config["speed_index"];
            mpIndex                       = config["mp_index"];
            config["mp_boundary_min"]     = mp_min;
            config["mp_boundary_max"]     = mp_max;
            config["mp_query_time"]       = 0.1;
            config["mp_connection_range"] = 1;
            m_config                      = config;
            setWorstMP();

            std::vector<std::string> grounded_action_names                     = config["tasks"];
            std::vector<std::vector<unsigned int>> tasks_start_end = config["tasks_start_end"];
            m_task_locations = boost::make_shared<std::vector<std::pair<unsigned int, unsigned int>>>();
            for(int i = 0; i < tasks_start_end.size(); ++i)
            {
                m_task_locations->push_back(std::pair(tasks_start_end[i][0], tasks_start_end[i][1]));
            }
            for(int i = 0; i < grounded_action_names.size(); ++i)
            {
                actionToRequirements[grounded_action_names[i]]  = i;
                m_action_location_map[grounded_action_names[i]] = (*m_task_locations)[i];
                actionNonCumRequirements.push_back({0.0, 0.0, 0.0, 0.0, 0.0});
            }
            actionRequirements = config["tasks_trait_requirements"].get<std::vector<std::vector<float>>>();
        }

        void IrosProblem::setWorstMP()
        {
            longestPath = 0;
            for(int i = 0; i < m_map.size(); i++)
            {
                float currentPath = 0;
                for(int j = 0; j < m_map[i].size(); j++)
                {
                    // std::cout << "New" << std::endl;
                    for(int k = 0; k < m_map[i][j].size(); k++)
                    {
                        if(k == ((m_map[i][j]).size() - 1))
                        {
                            float x_squared = pow(m_map[i][j][k].X - m_map[i][j][0].X, 2);
                            float y_squared = pow(m_map[i][j][k].Y - m_map[i][j][0].Y, 2);
                            currentPath += sqrt(x_squared + y_squared);
                        }
                        else
                        {
                            float x_squared =
                                pow(m_map[i][j][k].X - m_map[i][j][k + 1].X, 2);
                            float y_squared =
                                pow(m_map[i][j][k].Y - m_map[i][j][k + 1].Y, 2);
                            currentPath += sqrt(x_squared + y_squared);
                        }
                    }
                }
                if(currentPath > longestPath)
                {
                    longestPath = currentPath;
                }
            }
            if(speedIndex > 0)
            {
                float slowest = std::numeric_limits<float>::max();
                for(int i = 0; i < m_robot_traits.size(); i++)
                {
                    if(m_robot_traits[i][speedIndex] < slowest)
                    {
                        slowest = m_robot_traits[i][speedIndex];
                    }
                }
                longestPath += 4 * (mp_max - mp_min);
                longestPath = longestPath / slowest;
            }
        }


        void IrosProblem::setLocations(const std::vector<Location>& locations)
        {
            m_locations = locations;
        }

        void IrosProblem::setActionLocationMap(
            const std::map<std::string, std::pair<unsigned int, unsigned int>>& action_location_map)
        {
            m_action_location_map = action_location_map;
        }

        void IrosProblem::addActionLocation(const std::string& action,
                                        const std::pair<unsigned int, unsigned int>& location)
        {
            m_action_location_map[action] = location;
        }

        void IrosProblem::setActionRequirements(const std::vector<std::vector<float>>& actionReq)
        {
            actionRequirements = actionReq;
        }

        void IrosProblem::setActionNonCumRequirements(const std::vector<std::vector<float>>& actionNonCumReq)
        {
            actionNonCumRequirements = actionNonCumReq;
        }

        void IrosProblem::setStartingLocations(const std::vector<unsigned int>& starting_locations)
        {
            m_starting_locations = starting_locations;
        }

        void IrosProblem::setRobotTraitVector(const std::vector<IrosProblem::TraitVector>& robot_traits)
        {
            m_robot_traits = robot_traits;
        }


        void IrosProblem::setConfig(const nlohmann::json& config)
        {
            m_config = config;
        }

        const std::vector<Location>& IrosProblem::locations() const
        {
            return m_locations;
        }

        const Location& IrosProblem::location(uint i) const
        {
            return m_locations[i];
        }

        const std::pair<unsigned int, unsigned int>& IrosProblem::actionLocation(const std::string& name) const
        {
            if(m_action_location_map.find(name) != m_action_location_map.end())
            {
                return m_action_location_map.at(name);
            }
            throw "Unknown action";
        }

        std::vector<IrosProblem::TraitVector>& IrosProblem::robotTraits()
        {
            return m_robot_traits;
        }

        const IrosProblem::TraitVector& IrosProblem::robotTrait(uint i) const
        {
            return m_robot_traits[i];
        }

        const std::vector<unsigned int>& IrosProblem::startingLocations() const
        {
            return m_starting_locations;
        }

        const nlohmann::json& IrosProblem::config() const
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

        void convertToPaths(ClipperLib2::Paths& rv, const std::vector<std::vector<b2Vec2>>& shapes)
        {
            for(const std::vector<b2Vec2>& shape: shapes)
            {
                ClipperLib2::Path polygon;
                for(const b2Vec2& point: shape)
                {
                    polygon.push_back(ClipperLib2::IntPoint(point.x * 1E6, point.y * 1E6));
                }

                // If "hole" flip it
                const float area = ClipperLib2::Area(polygon);
                if(area < 0.0)
                {
                    std::reverse(polygon.begin(), polygon.end());
                }
                rv.push_back(polygon);
            }
        }

        ClipperLib2::Paths IrosProblem::convertBuildingsAndStreetsToPolygons2(const nlohmann::json& buildings,
                                                                          const nlohmann::json& streets)
        {
            ClipperLib2::Paths rv;
            convertToPaths(rv, buildings.get<std::vector<std::vector<b2Vec2>>>());
            convertToPaths(rv, streets.get<std::vector<std::vector<b2Vec2>>>());

            // union/smooth out
            {
                ClipperLib2::Clipper clipper;
                clipper.AddPaths(rv, ClipperLib2::ptSubject, true);
                clipper.Execute(ClipperLib2::ctUnion, rv, ClipperLib2::pftNonZero, ClipperLib2::pftNonZero);

                ClipperLib2::ClipperOffset clipper_offset;
                clipper_offset.AddPaths(rv, ClipperLib2::jtMiter, ClipperLib2::etClosedPolygon);
                clipper_offset.Execute(rv, 1E5);
                clipper_offset.Clear();
                clipper_offset.AddPaths(rv, ClipperLib2::jtMiter, ClipperLib2::etClosedPolygon);
                clipper_offset.Execute(rv, -1E5);
            }

            return rv;
        }

        void IrosProblem::writeMap(const std::string& folder)
        {
            nlohmann::json j;
            j["map"]       = m_map[0];
            j["locations"] = m_locations;

            std::ofstream output;
            output.open(fmt::format("{}/updated_map.json", folder).c_str());
            output << j.dump(4);
        }
        const std::vector<ClipperLib2::Paths>& IrosProblem::map() const
        {
            return m_map;
        }
        boost::shared_ptr<std::vector<std::vector<int>>>& IrosProblem::orderingConstraints()
        {
            return m_ordering_constraints;
        }
        boost::shared_ptr<std::vector<float>>& IrosProblem::durations()
        {
            return m_durations;
        }
        boost::shared_ptr<std::vector<std::vector<float>>>& IrosProblem::noncumTraitCutoff()
        {
            return m_noncum_trait_cutoff;
        }
        boost::shared_ptr<std::vector<std::vector<float>>>& IrosProblem::goalDistribution()
        {
            return m_goal_distribution;
        }
        boost::shared_ptr<std::vector<std::pair<unsigned int, unsigned int>>>& IrosProblem::taskLocations()
        {
            return m_task_locations;
        }

    }
}  // namespace grstaps
