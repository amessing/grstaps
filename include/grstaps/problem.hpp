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

#ifndef GRSTAPS_PROBLEM_HPP
#define GRSTAPS_PROBLEM_HPP

// global
#include <memory>
#include <vector>

// external
#include <../lib/unordered_map/robin_hood.h>
#include <box2d/b2_polygon_shape.h>
#include <nlohmann/json.hpp>

// local
#include "grstaps/location.hpp"
#include "grstaps/task_planning/sas_task.hpp"
#include "grstaps/task_planning/utils.hpp"

namespace grstaps
{
    /**
     * Creates a HMA-CTAMP problem
     */
    class Problem
    {
       public:
        using TraitVector        = std::vector<float>;
        using NonCumVector       = std::vector<float>;
        using RequirementsVector = std::vector<float>;

        Problem();
        void init(const char* domain, const char* problem, const char* parameters, const char* map_file);
        void configureActions(std::function<void(const std::vector<SASAction>&, Problem*)> configure_function);
        void setLocations(const std::vector<Location>& locations);
        void setActionLocationMap(
            const std::map<std::string, std::pair<unsigned int, unsigned int>>& action_location_map);
        void addActionLocation(const std::string& action, const std::pair<unsigned int, unsigned int>& location);
        void setRobotTraitVector(const std::vector<TraitVector>& robot_traits);
        void setTask(SASTask* task);
        void setObstacles(const std::vector<std::vector<b2PolygonShape>>& obstacles);
        void setConfig(const nlohmann::json& config);
        void setActionRequirements(const std::vector<std::vector<float>>& actionReq);
        void setActionNonCumRequirements(const std::vector<std::vector<float>>& actionNonCumReq);
        void setStartingLocations(const std::vector<unsigned int>& starting_locations);

        void setWorstMP();

        const std::vector<Location>& locations() const;
        const Location& location(uint i) const;
        const std::pair<unsigned int, unsigned int>& actionLocation(const std::string& name) const;
        std::vector<TraitVector>& robotTraits();
        const TraitVector& robotTrait(uint i) const;
        const SASTask* task() const;
        const std::vector<std::vector<b2PolygonShape>> obstacles() const;
        const std::vector<unsigned int>& startingLocations() const;
        const nlohmann::json& config() const;
        SASTask* task();

        void writeMap(const std::string& folder);

        robin_hood::unordered_map<std::string, int> actionToRequirements;  //!< Unordered_map to the graphs edges

        std::vector<RequirementsVector> actionRequirements;
        std::vector<NonCumVector> actionNonCumRequirements;
        int speedIndex;
        int mpIndex;
        float longestPath;
        float mp_max;
        float mp_min;

       protected:
        std::vector<b2PolygonShape> convertBuildingsAndStreetsToPolygons(const nlohmann::json& buildings, const nlohmann::json& streets);

        std::vector<Location> m_locations;  //!< coordinates and name of location
        std::map<std::string, std::pair<unsigned int, unsigned int>>
            m_action_location_map;                       // mapping of actions to their start and end locations
        std::vector<TraitVector> m_robot_traits;         //!< List of vectors of robot traits
        std::vector<unsigned int> m_starting_locations;  //!< List of the starting location of the robots
        SASTask* m_task;                                 // A SAS Task for the Task planner
        std::vector<std::vector<b2PolygonShape>> m_map;         //!< Obstacles in the map
        nlohmann::json m_config;
    };
}  // namespace grstaps

#endif  // GRSTAPS_PROBLEM_HPP
