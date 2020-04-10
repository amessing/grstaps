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
 *//*
#include "grstaps/problem.hpp"

// global
#include <fstream>
#include <random>

// local
#include "grstaps/constants.hpp"

namespace grstaps
{

    std::shared_ptr<Problem> Problem::createSurvivorProblem(const nlohmann::json& config)
    {
        // Used to obtain a seed for the random number engine
        std::random_device rd;

        // Standard mersenne_twister_engine seeded with rd()
        std::mt19937 gen(rd());

        // Build Locations
        std::uniform_int_distribution<> num_hospital_generator(1, config["max_num_hospital"]);
        unsigned int num_hospital = num_hospital_generator(gen);

        std::uniform_int_distribution<> num_survivor_generator(1, config["max_num_survivors"]);
        unsigned int num_survivor = num_survivor_generator(gen);

        std::uniform_int_distribution<> num_fire_generator(1, config["max_num_fire"]);
        unsigned int num_fires = num_fire_generator(gen);

        // Build Objects
        for(int i = 0; i < num_survivor; ++i)
        {
            nlohmann::json properties = {
                {}
            };

            auto survivor = std::make_shared<Object>(constants::k_survivor, );
        }

        // Build Actions

        // Build Robots
    }

    void Problem::write(const std::string& filepath)
    {
        std::ofstream file(filepath);
        file << m_config.dump(4);
    }

    const Action& Problem::action(unsigned int id) const
    {
        return m_actions[id];
    }

    const Location& Problem::location(unsigned int id) const
    {
        return m_locations[id];
    }

    const Object& Problem::object(unsigned int id) const
    {
        return m_objects[id];
    }

    const Robot& Problem::robot(unsigned int id) const
    {
        return m_robot[id];
    }
}
*/