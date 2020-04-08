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
#include <nlohmann/json.hpp>

namespace grstaps
{
    // Forward Declarations
    class Action;
    class Location;
    class Object;
    class Robot;

    class Problem
    {
    public:
        /**
         * Factory function to create a Problem
         *
         * \param config The configuration for generating a problem
         */
        static std::shared_ptr<Problem> createSurvivorProblem(const nlohmann::json& config);

        /**
         * Writes the problem config
         */
        void write(const std::string& filepath);

        /**
         * \returns The action specified by the \p id
         */
        const Action& action(unsigned int id) const;

        /**
         * \returns The location specified by the \p id
         */
        const Location& location(unsigned int id) const;

        /**
         * \returns The object specified by the \p id
         */
        const Object& object(unsigned int id) const;

        /**
         * \returns The robot specified by the \p id
         */
        const Robot& robot(unsigned int id) const;

    private:
        /**
         * Default Constructor
         */
        Problem() = default;

        std::vector<Action> m_actions;
        std::vector<Location> m_locations;
        std::vector<Object> m_objects;
        std::vector<Robot> m_robot;

        nlohmann::json m_config;
    };
}

#endif //GRSTAPS_PROBLEM_HPP
