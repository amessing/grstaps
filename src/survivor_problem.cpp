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
#include "grstaps/survivor_problem.hpp"

// local
#include "grstaps/constants.hpp"
#include "grstaps/location.hpp"
#include "grstaps/logger.hpp"

namespace grstaps
{
    std::unique_ptr<std::random_device> SurvivorProblem::s_rd = nullptr;
    std::unique_ptr<std::mt19937> SurvivorProblem::s_generator = nullptr;

    void SurvivorProblem::init(const nlohmann::json& config)
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
        createLocations(config);
        createInitialState(config);
        createGoal(config);
        //createActions(config, vars);

    }

    void SurvivorProblem::buildMap(const nlohmann::json& config)
    {
        // Create a grid

        // Add warehouse boundary

        // Add obstacles (premade shapes)
    }

    void SurvivorProblem::createLocations(const nlohmann::json& config)
    {
        // Add medicine start location
        std::pair<unsigned int, unsigned int> xy = createMedicineStartLocation();
        m_state_decoder->m_medicine_start = m_locations.size();
        m_locations.emplace_back(constants::k_medicine_location, xy.first, xy.second);

        // Add water start location
        xy = createWaterStartLocation();
        m_state_decoder->m_water_start = m_locations.size();
        m_locations.emplace_back(constants::k_water_location, xy.first, xy.second);

        // Add small crate packing location
        xy = createSmallCrateStartLocation();
        m_state_decoder->m_small_crate_start = m_locations.size();
        m_locations.emplace_back(constants::k_small_crate_location, xy.first, xy.second);

        // Add large crate packing location
        xy = createLargeCrateStartLocation();
        m_state_decoder->m_large_crate_start = m_locations.size();
        m_locations.emplace_back(constants::k_large_crate_location, xy.first, xy.second);

        // Add construction kit start location
        xy = createConstructionKitStartLocation();
        m_state_decoder->m_construction_kit_start = m_locations.size();
        m_locations.emplace_back(constants::k_construction_kit_location, xy.first, xy.second);

        // Add pickup location
        xy = createPickupLocation();
        m_state_decoder->m_pickup = m_locations.size();
        m_locations.emplace_back(constants::k_pickup_location, xy.first, xy.second);

        // Add hospital locations
        {
            m_state_decoder->m_num_hospitals = randomUnsignedInt(config[constants::k_min_num_hospitals],
                                                                 config[constants::k_max_num_hospitals]);
            m_state_decoder->m_first_hospital = m_locations.size();
            Logger::debug("Adding {0:d} hospitals", m_state_decoder->m_num_hospitals);
            for(uint16_t i = 0; i < m_state_decoder->m_num_hospitals; ++i)
            {
                xy = createLocationOutsideWarehouse();
                m_locations.emplace_back(fmt::format("{0:s}_{1:d}", constants::k_hospital_location, i + 1),
                                         xy.first,
                                         xy.second);
            }
        }

        // Add survivor locations
        {
            m_state_decoder->m_num_survivors = randomUnsignedInt(config[constants::k_min_num_survivors],
                                                                 config[constants::k_max_num_survivors]);
            m_state_decoder->m_first_survivor_loc = m_locations.size();
            Logger::debug("Adding {0:d} survivors", m_state_decoder->m_num_survivors);
            for(unsigned int i = 0; i < m_state_decoder->m_num_survivors; ++i)
            {
                xy = createLocationOutsideWarehouse();
                m_locations.emplace_back(fmt::format("{0:s}_{1:d}", constants::k_survivor_location, i + 1),
                                         xy.first,
                                         xy.second);
            }
        }

        // Add fire locations
        {
            m_state_decoder->m_num_fires = randomUnsignedInt(config[constants::k_min_num_fire],
                                                       config[constants::k_max_num_fire]);
            m_state_decoder->m_first_fire_loc = m_locations.size();
            Logger::debug("Adding {0:d} fires", m_state_decoder->m_num_fires);
            for(unsigned int i = 0; i < m_state_decoder->m_num_fires; ++i)
            {
                xy = createLocationOutsideWarehouse();
                m_locations.emplace_back(fmt::format("{0:s}_{1:d}", constants::k_fire_location, i + 1),
                                         xy.first,
                                         xy.second);
            }
        }

        // Add damaged building locations
        {
            m_state_decoder->m_num_damaged_buildings = randomUnsignedInt(config[constants::k_min_num_damaged_buildings],
                                                                   config[constants::k_max_num_damaged_buildings]);
            m_state_decoder->m_first_damaged_building_loc = m_locations.size();
            Logger::debug("Adding {0:d} damaged buildings", m_state_decoder->m_num_damaged_buildings);
            for(unsigned int i = 0; i < m_state_decoder->m_num_damaged_buildings; ++i)
            {
                xy = createLocationOutsideWarehouse();
                m_locations.emplace_back(fmt::format("{0:s}_{1:d}", constants::k_damaged_building_location, i + 1),
                                         xy.first,
                                         xy.second);
            }
        }

        m_state_decoder->m_total_locations = m_locations.size();
    }

    void SurvivorProblem::createInitialState(const nlohmann::json& config)
    {
        Logger::debug("Creating initial state");
        addInitialLocationsToInitialState(config);
        unsigned int state_variable = m_state_decoder->m_total_locations;

        // Record empty state assignment for each medicine
        m_state_decoder->m_first_empty = m_initial_state.size();
        for(unsigned int i = 0; i < m_state_decoder->m_num_medicine; ++i, ++state_variable)
        {
            Logger::debug("({0:d}) Medicine {1:d} empty - {2:d}", state_variable, i + 1, 0);
            m_initial_state.emplace_back(state_variable, 0);
        }

        // Record contains state assignment for each crate
        m_state_decoder->m_first_contains = m_initial_state.size();
        for(unsigned int i = 0; i < m_state_decoder->m_num_small_crate; ++i, ++state_variable)
        {
            Logger::debug("({0:d}) Small Crate {1:d} contains - {2:d}", state_variable, i + 1, 0);
            m_initial_state.emplace_back(state_variable, 0);
        }
        for(unsigned int i = 0; i < m_state_decoder->m_num_large_crate; ++i, ++state_variable)
        {
            Logger::debug("({0:d}) Large Crate {1:d} contains - {2:d}", state_variable, i + 1, 0);
            m_initial_state.emplace_back(state_variable, 0);
        }

        // Record in state assignment for each medicine
        m_state_decoder->m_first_in = m_initial_state.size();
        for(unsigned int i = 0; i < m_state_decoder->m_num_medicine; ++i, ++state_variable)
        {
            Logger::debug("({0:d}) Medicine {1:d} in - {2:d}", state_variable, i + 1, 0);
            m_initial_state.emplace_back(state_variable, 0);
        }

        // Record uses state assignment for each water container
        m_state_decoder->m_first_uses = m_initial_state.size();
        for(unsigned int i = 0; i < m_state_decoder->m_num_small_water_container; ++i, ++state_variable)
        {
            Logger::debug("({0:d}) Small Water Container {1:d} uses - {2:d}", state_variable, i + 1, 0);
            m_initial_state.emplace_back(state_variable, 0);
        }
        for(unsigned int i = 0; i < m_state_decoder->m_num_large_water_container; ++i, ++state_variable)
        {
            Logger::debug("({0:d}) Large Water Container {1:d} uses - {2:d}", state_variable, i + 1, 0);
            m_initial_state.emplace_back(state_variable, 0);
        }

        // Record onFire status for each fire and damaged building
        m_state_decoder->m_first_on_fire = m_initial_state.size();
        for(unsigned int i = 0; i < m_state_decoder->m_num_fires; ++i, ++state_variable)
        {
            Logger::debug("({0:d}) Fire {1:d} onFire - {2:d}", state_variable, i + 1, 0);
            m_initial_state.emplace_back(state_variable, 1);
        }
        for(unsigned int i = 0; i < m_state_decoder->m_num_damaged_buildings; ++i, ++state_variable)
        {
            Logger::debug("({0:d}) Damaged Building {1:d} onFire - {2:d}", state_variable, i + 1, 0);
            m_initial_state.emplace_back(state_variable, i < m_state_decoder->m_num_damaged_buildings_on_fire ? 1 : 0);
        }

        // Record rubbleCleared status for each damaged building
        m_state_decoder->m_first_rubble_cleared = m_initial_state.size();
        for(unsigned int i = 0; i < m_state_decoder->m_num_damaged_buildings; ++i, ++state_variable)
        {
            Logger::debug("({0:d}) Damaged Building {1:d} rubbleCleared - {2:d}", state_variable, i + 1, 0);
            m_initial_state.emplace_back(state_variable, 0);
        }

        // Record repaired status for each damaged building
        m_state_decoder->m_first_repaired = m_initial_state.size();
        for(unsigned int i = 0; i < m_state_decoder->m_num_damaged_buildings; ++i, ++state_variable)
        {
            Logger::debug("({0:d}) Damaged Building {1:d} repaired - {2:d}", state_variable, i + 1, 0);
            m_initial_state.emplace_back(state_variable, 0);
        }

        // Record healed status for each survivor
        m_state_decoder->m_first_healed = m_initial_state.size();
        for(unsigned int i = 0; i < m_state_decoder->m_num_survivors; ++i, ++state_variable)
        {
            Logger::debug("({0:d}) Survivor {1:d} healed - {2:d}", state_variable, i + 1, 0);
            m_initial_state.emplace_back(state_variable, 0);
        }
    }

    void SurvivorProblem::addInitialLocationsToInitialState(const nlohmann::json& config)
    {
        unsigned int num_survivors = m_state_decoder->m_num_survivors;
        unsigned int state_variable = 0;

        // Record initial locations of all objects
        Logger::debug("Recording initial locations");
        //// Record locations for each medicine vial
        unsigned int num_medicine;
        {
            num_medicine = randomUnsignedInt(num_survivors,
                                             num_survivors + config[constants::k_extra_medicine].get<unsigned int>());
            Logger::debug("Adding {0:d} medicine", num_medicine);
            for(unsigned int i = 0; i < num_medicine; ++i, ++state_variable)
            {
                Logger::debug("({0:d}) Medicine {1:d} - {2:s}",
                              state_variable,
                              i + 1,
                              m_locations[m_state_decoder->m_medicine_start].type());
                m_initial_state.emplace_back(state_variable, m_state_decoder->m_medicine_start);
            }
        }

        //// Record locations for each crate
        float crate_ratio = randomFloat();
        {
            m_state_decoder->m_num_small_crate = static_cast<unsigned int>(num_medicine * crate_ratio / 5.0) + 1 +
                                           randomUnsignedInt(0, config[constants::k_extra_small_crates]);
            Logger::debug("Adding {0:d} small crates", m_state_decoder->m_num_small_crate);
            for(unsigned int i = 0; i < m_state_decoder->m_num_small_crate; ++i, ++state_variable)
            {
                Logger::debug("({0:d}) Small Crate {1:d} - {2:s}",
                              state_variable,
                              i + 1,
                              m_locations[m_state_decoder->m_small_crate_start].type());
                m_initial_state.emplace_back(state_variable, m_state_decoder->m_small_crate_start);
            }
        }

        {
            m_state_decoder->m_num_large_crate = static_cast<unsigned int>(num_medicine * (1.0 - crate_ratio) / 10.0) + 1 +
                                           randomUnsignedInt(0, config[constants::k_extra_large_crates]);
            Logger::debug("Adding {0:d} large crates", m_state_decoder->m_num_large_crate );
            for(unsigned int i = 0; i < m_state_decoder->m_num_large_crate ; ++i, ++state_variable)
            {
                Logger::debug("({0:d}) Small Crate {1:d} - {2:s}",
                              state_variable,
                              i + 1,
                              m_locations[m_state_decoder->m_large_crate_start].type());
                m_initial_state.emplace_back(state_variable, m_state_decoder->m_large_crate_start);
            }
        }

        //// Record locations for each water container
        float damaged_building_fire_ratio = randomFloat();
        unsigned int num_damaged_buildings = m_state_decoder->m_num_damaged_buildings
        unsigned int num_damaged_building_fire = damaged_building_fire_ratio * num_damaged_buildings;
        Logger::debug("{0:d} of {1:d} the damaged buildings are on FIRE",
                      num_damaged_building_fire,
                      num_damaged_buildings);
        {
            unsigned int num_total_fire = m_state_decoder->m_num_fires + num_damaged_building_fire;
            float small_water_ratio = randomFloat();
            m_state_decoder->m_num_small_water_container = static_cast<unsigned int>(num_total_fire * small_water_ratio) + 1 +
                                           randomUnsignedInt(0, config[constants::k_extra_small_water_containers]);
            Logger::debug("Adding {0:d} small water containers", m_state_decoder->m_num_small_water_container);
            for(unsigned int i = 0; i < m_state_decoder->m_num_small_water_container; ++i, ++state_variable)
            {
                Logger::debug("({0:d}) Small Water {1:d} - {2:s}",
                              state_variable,
                              i + 1,
                              m_locations[m_state_decoder->m_water_start].type());
                m_initial_state.emplace_back(state_variable, m_state_decoder->m_water_start);
            }

            m_state_decoder->m_num_large_water_container =
                static_cast<unsigned int>(num_total_fire * (1.0 - small_water_ratio) / 2.0) + 1 +
                randomUnsignedInt(0, config[constants::k_extra_large_water_containers]);
            Logger::debug("Adding {0:d} large water containers", m_state_decoder->m_num_large_water_container);
            for(unsigned int i = 0; i < m_state_decoder->m_num_large_water_container; ++i, ++state_variable)
            {
                Logger::debug("({0:d}) Large Water {1:d} - {2:s}",
                              state_variable,
                              i + 1,
                              m_locations[m_state_decoder->m_water_start].type());
                m_initial_state.emplace_back(state_variable, m_state_decoder->m_water_start);
            }
        }

        //// Record locations for each survivor
        unsigned int first_survivor_location = m_state_decoder->m_first_survivor_loc;
        for(unsigned int i = 0; i < num_survivors; ++i, ++state_variable)
        {
            Logger::debug("({0:d}) Survivor {1:d} - {2:s}",
                          state_variable,
                          i + 1,
                          m_locations[first_survivor_location + i].type());
            m_initial_state.emplace_back(state_variable, first_survivor_location + i);
        }

        //// Record locations for each construction kit
        for(unsigned int i = 0; i < num_survivors; ++i, ++state_variable)
        {
            Logger::debug("({0:d}) Survivor {1:d} - {2:s}",
                          state_variable,
                          i + 1,
                          m_locations[m_state_decoder->m_construction_kit_start].type());
            m_initial_state.emplace_back(state_variable, m_state_decoder->m_construction_kit_start);
        }
    }

    void SurvivorProblem::createGoal(const nlohmann::json& config)
    {
        Logger::debug("Creating goal");

        // Each survivor healed
        for(unsigned int i = 0; i < m_state_decoder->m_num_survivors; ++i)
        {
            Logger::debug("({0:d}) Survivor {1:d} - {2:d}",
                          m_state_decoder->m_first_healed + i,
                          i + 1,
                          1);
            m_goal.emplace_back(m_state_decoder->m_first_healed + i, 1);
        }

        // Each fire extinguished
        for(unsigned int i = 0; i < m_state_decoder->m_num_fires; ++i)
        {
            Logger::debug("({0:d}) Fire {1:d} onFire - {2:d}",
                          m_state_decoder->m_first_on_fire + i,
                          i + 1,
                          1);
            m_goal.emplace_back(m_state_decoder->m_first_on_fire + i, 0);
        }

        // Each damaged building repaired
        for(unsigned int i = 0; i < m_state_decoder->m_num_damaged_buildings; ++i)
        {
            Logger::debug("({0:d}) Damaged Building {1:d} repaired - {2:d}",
                          m_state_decoder->m_first_on_fire + i,
                          i + 1,
                          1);
            m_goal.emplace_back(m_state_decoder->m_first_repaired + i, 1);
        }
    }

    void SurvivorProblem::createActions(const nlohmann::json& config)
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
