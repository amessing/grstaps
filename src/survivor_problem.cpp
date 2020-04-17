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
#if 0
#include "grstaps/survivor_problem.hpp"

// local
#include "grstaps/constants.hpp"
#include "grstaps/location.hpp"
#include "grstaps/logger.hpp"

namespace grstaps
{
    std::unique_ptr<std::random_device> SurvivorProblem::s_rd = nullptr;
    std::unique_ptr<std::mt19937> SurvivorProblem::s_generator = nullptr;

    void SurvivorProblem::init(const nlohmann::json& config, SurvivorStateDecoder& state_decoder)
    {
        Logger::debug("Building a survivor problem");

        static bool first = true;
        if(first)
        {
            // Used to obtain a seed for the random number engine
            s_rd = std::make_unique<std::random_device>();

            // Standard mersenne_twister_engine seeded with rd()
            s_generator = std::make_unique<std::mt19937>(s_rd->operator()());
            first = false;
        }

        buildMap(config);

        nlohmann::json vars;
        //createLocations(config, vars);
        //createInitialState(config, vars);
        //createGoal(config, vars);
        //createActions(config, vars);

    }

    void SurvivorProblem::buildMap(const nlohmann::json& config)
    {
        // Create a grid

        // Add warehouse boundary

        // Add obstacles (premade shapes)
    }

    void SurvivorProblem::createLocations(const nlohmann::json& config, SurvivorStateDecoder& state_decoder)
    {
        // Add medicine start location
        std::pair<unsigned int, unsigned int> xy = createMedicineStartLocation();
        state_decoder.m_location_type_start_indicies[state_decoder.k_medicine_start] = m_locations.size();
        m_locations.emplace_back(constants::k_medicine_location, xy.first, xy.second);


        // Add water start location
        xy = createWaterStartLocation();
        state_decoder.m_location_type_start_indicies[state_decoder.k_water_start] = m_locations.size();
        m_locations.emplace_back(constants::k_water_location, xy.first, xy.second);

        // Add small crate packing location
        xy = createSmallCrateStartLocation();
        state_decoder.m_location_type_start_indicies[state_decoder.k_small_crate_start] = m_locations.size();
        m_locations.emplace_back(constants::k_small_crate_location, xy.first, xy.second);

        // Add large crate packing location
        xy = createLargeCrateStartLocation();
        state_decoder.m_location_type_start_indicies[state_decoder.k_large_crate_start] = m_locations.size();
        m_locations.emplace_back(constants::k_large_crate_location, xy.first, xy.second);

        // Add construction kit start location
        xy = createConstructionKitStartLocation();
        state_decoder.m_location_type_start_indicies[state_decoder.k_construction_kit_start] = m_locations.size();
        m_locations.emplace_back(constants::k_construction_kit_location, xy.first, xy.second);

        // Add pickup location
        xy = createPickupLocation();
        state_decoder.m_location_type_start_indicies[state_decoder.k_pickup] = m_locations.size();
        m_locations.emplace_back(constants::k_pickup_location, xy.first, xy.second);

        // Add hospital locations
        {
            unsigned int num_hospitals = randomUnsignedInt(config[constants::k_min_num_hospitals],
                                                           config[constants::k_max_num_hospitals]);
            state_decoder.m_location_type_start_indicies[state_decoder.k_hospital] = m_locations.size();
            Logger::debug("Adding {0:d} hospitals", num_hospitals);
            for(unsigned int i = 0; i < num_hospitals; ++i)
            {
                xy = createLocationOutsideWarehouse();
                m_locations.emplace_back(fmt::format("{0:s}_{1:d}", constants::k_hospital_location, i + 1), xy.first, xy.second);
            }
        }

        // Add survivor locations
        {
            unsigned int num_survivors = randomUnsignedInt(config[constants::k_min_num_survivors],
                                                           config[constants::k_max_num_survivors]);
            state_decoder.m_location_type_start_indicies[state_decoder.k_survivor] = m_locations.size();
            Logger::debug("Adding {0:d} survivors", num_survivors);
            for(unsigned int i = 0; i < num_survivors; ++i)
            {
                xy = createLocationOutsideWarehouse();
                m_locations.emplace_back(fmt::format("{0:s}_{1:d}",constants::k_survivor_location, i + 1), xy.first, xy.second);
            }
        }

        // Add fire locations
        {
            unsigned int num_fires = randomUnsignedInt(config[constants::k_min_num_fire],
                                                       config[constants::k_max_num_fire]);
            state_decoder.m_location_type_start_indicies[state_decoder.k_fire] = m_locations.size();
            Logger::debug("Adding {0:d} fires", num_fires);
            for(unsigned int i = 0; i < num_fires; ++i)
            {
                xy = createLocationOutsideWarehouse();
                m_locations.emplace_back(fmt::format("{0:s}_{1:d}", constants::k_fire_location, i + 1), xy.first, xy.second);
            }
        }

        // Add damaged building locations
        {
            unsigned int num_damaged_buildings = randomUnsignedInt(config[constants::k_min_num_damaged_buildings],
                                                                   config[constants::k_max_num_damaged_buildings]);
            state_decoder.m_location_type_start_indicies[state_decoder.k_damaged_building] = m_locations.size();
            Logger::debug("Adding {0:d} damaged buildings", num_damaged_buildings);
            for(unsigned int i = 0; i < num_damaged_buildings; ++i)
            {
                xy = createLocationOutsideWarehouse();
                m_locations.emplace_back(fmt::format("{0:s}_{1:d}", constants::k_damaged_building_location, i + 1), xy.first, xy.second);
            }
        }

        state_decoder.m_location_type_start_indicies[state_decoder.k_total_locations] = m_locations.size();
    }

    void SurvivorProblem::createInitialState(const nlohmann::json& config,
                                             SurvivorStateDecoder& state_decoder)
    {
        Logger::debug("Creating initial state");
        unsigned int state_variable = 0;
        unsigned int num_survivors = state_decoder.m_location_type_start_indicies[state_decoder.k_fire] - state_decoder.m_location_type_start_indicies[state_decoder.k_survivor];

        // Record initial locations of all objects
        Logger::debug("Recording initial locations");
        //state_decoder.
        //// Record locations for each medicine vial
        unsigned int num_medicine;
        {
            num_medicine = randomUnsignedInt(num_survivors,
                                             num_survivors +
                                             config[constants::k_extra_medicine].get<unsigned int>());
            Logger::debug("Adding {0:d} medicine", num_medicine);
            for(unsigned int i = 0; i < num_medicine; ++i, ++state_variable)
            {
                Logger::debug("({0:d}) Medicine {1:d} - {2:s}",
                              state_variable,
                              i + 1,
                              m_locations[constants::k_medicine_start_location_index].type());
                m_initial_state.emplace_back(state_variable, constants::k_medicine_start_location_index);
            }
        }

        //// Record locations for each crate
        float crate_ratio = randomFloat();
        {
            unsigned int num_small_crate =
                static_cast<unsigned int>(num_medicine * crate_ratio / 5.0) + 1 + randomUnsignedInt(0, config[constants::k_extra_small_crates]);
            Logger::debug("Adding {0:d} small crates", num_small_crate);
            for(unsigned int i = 0; i < num_small_crate; ++i, ++state_variable)
            {
                Logger::debug("({0:d}) Small Crate {1:d} - {2:s}",
                              state_variable,
                              i + 1,
                              m_locations[constants::k_small_crate_location_index].type());
                m_initial_state.emplace_back(state_variable, constants::k_small_crate_location_index);
            }
        }

        {
            unsigned int num_large_crate = static_cast<unsigned int>(num_medicine * (1.0 - crate_ratio) / 10.0) + 1 +
                                           randomUnsignedInt(0, config[constants::k_extra_large_crates]);
            Logger::debug("Adding {0:d} large crates", num_large_crate);
            for(unsigned int i = 0; i < num_large_crate; ++i, ++state_variable)
            {
                Logger::debug("({0:d}) Small Crate {1:d} - {2:s}",
                              state_variable,
                              i + 1,
                              m_locations[constants::k_large_crate_location_index].type());
                m_initial_state.emplace_back(state_variable, constants::k_large_crate_location_index);
            }
        }

        //// Record locations for each water container
        float damaged_building_fire_ratio = randomFloat();
        unsigned int num_damaged_buildings = vars[constants::k_num_damaged_buildings];
        unsigned int num_damaged_building_fire = damaged_building_fire_ratio * num_damaged_buildings;
        Logger::debug("{0:d} of {1:d} the damaged buildings are on FIRE", num_damaged_building_fire, num_damaged_buildings);
        {
            unsigned int num_total_fire = vars[constants::k_num_fires].get<unsigned int>() + num_damaged_building_fire;
            float small_water_ratio = randomFloat();
            unsigned int num_small_water = static_cast<unsigned int>(num_total_fire * small_water_ratio) + 1 + randomUnsignedInt(0, config[constants::k_extra_small_water_containers]);
            Logger::debug("Adding {0:d} small water containers", num_small_water);
            for(unsigned int i = 0; i < num_small_water; ++i, ++state_variable)
            {
                Logger::debug("({0:d}) Small Water {1:d} - {2:s}", state_variable,
                              i + 1,
                              m_locations[constants::k_water_start_location_index].type());
                m_initial_state.emplace_back(state_variable, constants::k_water_start_location_index);
            }

            unsigned int num_large_water = static_cast<unsigned int>(num_total_fire * (1.0 - small_water_ratio) / 2.0) + 1 + randomUnsignedInt(0, config[constants::k_extra_large_water_containers]);
            Logger::debug("Adding {0:d} large water containers", num_large_water);
            for(unsigned int i = 0; i < num_large_water; ++i, ++state_variable)
            {
                Logger::debug("({0:d}) Large Water {1:d} - {2:s}", state_variable,
                              i + 1,
                              m_locations[constants::k_water_start_location_index].type());
                m_initial_state.emplace_back(state_variable, constants::k_water_start_location_index);
            }
        }

        //// Record locations for each survivor
        unsigned int first_survivor_location = vars[constants::k_first_survivor_location_index];
        for(unsigned int i = 0; i < num_survivors; ++i, ++state_variable)
        {
            Logger::debug("({0:d}) Survivor {1:d} - {2:s}", state_variable,
                          i + 1,
                          m_locations[first_survivor_location + i].type());
            m_initial_state.emplace_back(state_variable, first_survivor_location + i);
        }

        //// Record locations for each construction kit


        // Record used state assignment for each medicine
        vars[constants::k_used_first_index] = m_initial_state.size();
        for(unsigned int i = 0; i < num_medicine; ++i, ++state_variable)
        {
            Logger::debug("({0:d}) Medicine {1:d} used - {2:d}", state_variable,
                          i + 1,
                          0);
            m_initial_state.emplace_back(state_variable, 0);
        }

        // Record contains state assignment for each crate
        vars[constants::k_contains_first_index] = m_initial_state.size();


        // Record in state assignment for each medicine
        vars[constants::k_in_first_index] = m_initial_state.size();

        // Record uses state assignment for each water container
        vars[constants::k_uses_first_index] = m_initial_state.size();

        // Record onFire status for each fire and damaged building
        vars[constants::k_on_fire_first_index] = m_initial_state.size();

        // Record rubbleCleared status for each damaged building
        vars[constants::k_rubbed_cleared_first_index] = m_initial_state.size();

        // Record repaired status for each damaged building
        vars[constants::k_repaired_first_index] = m_initial_state.size();

        // Record healed status for each survivor
        vars[constants::k_healed_first_index] = m_initial_state.size();
    }

    void SurvivorProblem::createGoal(const nlohmann::json& config, nlohmann::json& vars)
    {
        // todo
    }

    void SurvivorProblem::createActions(const nlohmann::json& config, nlohmann::json& vars)
    {
        // todo
    }

    unsigned int SurvivorProblem::randomUnsignedInt(unsigned int min, unsigned int max)
    {
        std::uniform_int_distribution<> rand(min, max);
        return rand(*s_generator);
    }

    float SurvivorProblem::randomFloat(float min, float max)
    {
        std::uniform_real_distribution<> rand(min, max);
        return rand(*s_generator);
    }

    float SurvivorProblem::randomFloat()
    {
        return randomFloat(0.0, 1.0);
    }

    SurvivorProblem::Coordinates SurvivorProblem::createLocationOutsideWarehouse()
    {
        // todo
        return std::make_pair<float, float>(0.0f, 0.0f);
    }

    SurvivorProblem::Coordinates SurvivorProblem::createMedicineStartLocation()
    {
        // todo
        return std::make_pair<float, float>(0.0f, 0.0f);
    }

    SurvivorProblem::Coordinates SurvivorProblem::createWaterStartLocation()
    {
        // todo
        return std::make_pair<float, float>(0.0f, 0.0f);
    }

    SurvivorProblem::Coordinates SurvivorProblem::createSmallCrateStartLocation()
    {
        // todo
        return std::make_pair<float, float>(0.0f, 0.0f);
    }

    SurvivorProblem::Coordinates SurvivorProblem::createLargeCrateStartLocation()
    {
        // todo
        return std::make_pair<float, float>(0.0f, 0.0f);
    }

    SurvivorProblem::Coordinates SurvivorProblem::createPickupLocation()
    {
        // todo
        return std::make_pair<float, float>(0.0f, 0.0f);
    }
}
#endif