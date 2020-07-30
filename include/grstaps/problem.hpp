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
#include <box2d/b2_polygon_shape.h>
#include <nlohmann/json.hpp>
#include <../lib/unordered_map/robin_hood.h>


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
        using TraitVector = std::vector<float>;
        using NonCumVector = std::vector<float>;
        using RequirementsVector = std::vector<float>;

        Problem() = default;
        void setLocations(const std::vector<Location>& locations);
        void setRobotTraitVector(std::vector<TraitVector>& robot_traits);
        void setTask(SASTask* task);
        void setObstacles(const std::vector<b2PolygonShape>& obstacles);
        void setConfig(const nlohmann::json& config);
        void setActionRequirements(const std::vector<std::vector<float>>& actionReq);
        void setActionNonCumRequirements(const std::vector<std::vector<float>>& actionNonCumReq);

        const std::vector<Location>& locations() const;
        const Location& location(uint i) const;
        std::vector<TraitVector>& robotTraits();
        const TraitVector& robotTrait(uint i) const;
        const SASTask* task() const;
        const std::vector<b2PolygonShape>& obstacles() const;
        const nlohmann::json& config() const;
        robin_hood::unordered_map<std::string, int> actionToRequirements; //!< Unordered_map to the graphs edges

        SASTask* task();
        std::vector<RequirementsVector> actionRequirements;
        std::vector<NonCumVector> actionNonCumRequirements;

    protected:
        std::vector<Location> m_locations; //!< coordinates and name of location
        std::vector<TraitVector> m_robot_traits; //!< List of vectors of robot traits
        SASTask* m_task; // A SAS Task for the Task planner
        std::vector<b2PolygonShape> m_obstacles; //!< Obstacles in the map
        nlohmann::json m_config;

    };
}

#endif //GRSTAPS_PROBLEM_HPP
