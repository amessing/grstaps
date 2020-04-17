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
#ifndef GRSTAPS_SURVIVOR_STATE_DECODER_HPP
#define GRSTAPS_SURVIVOR_STATE_DECODER_HPP

// global
#include <array>

// local
#include "grstaps/state_decoder.hpp"

namespace grstaps
{
    /**
     * Decodes a state from the survivor problem in a human readable format
     */
    class SurvivorStateDecoder : public StateDecoder
    {
    public:
        SurvivorStateDecoder() = default;

        /**
         * Prints a decoded version of the state to the screen
         */
        virtual void decode(const StateAssignment& state) override;

    private:
        /**
         * \returns The location type for the given index
         */
        std::string locationIndexToString(int16_t index);

        // Locations
        unsigned int m_medicine_start; //!< Index of the starting location for the medicine
        unsigned int m_water_start; //!< Index of the starting location of the water containers
        unsigned int m_small_crate_start; //!< Index of the starting location for the small crates
        unsigned int m_large_crate_start; //!< Index of the starting location for the large crates
        unsigned int m_construction_kit_start; //!< Index of the starting location for the construction kits
        unsigned int m_pickup; //!< Index of the pickup location
        unsigned int m_first_hospital; //!< Index of the first hospital location
        unsigned int m_num_hospitals; //!< Number of hospitals
        unsigned int m_first_survivor_loc; //!< Index of the first survivor location
        unsigned int m_num_survivors; //!< Number of survivors
        unsigned int m_first_fire_loc; //!< Index of the first fire location
        unsigned int m_num_fires; //!< Number of fires
        unsigned int m_first_damaged_building_loc; //!< Index of the first damaged building location
        unsigned int m_num_damaged_buildings; //!< Number of damaged buildings
        unsigned int m_total_locations; //!< The total number of locations

        // Number of objects
        unsigned int m_num_medicine; //!< Number of medicine
        unsigned int m_num_small_crate; //!< Number of small crates
        unsigned int m_num_large_crate; //!< Number of large crates
        unsigned int m_num_small_water_container; //!< Number of small water containers
        unsigned int m_num_large_water_container; //!< Number of large water containers
        unsigned int m_num_damaged_buildings_on_fire; //!< Number of damaged buildings that start on fire

        // Predicates/Functions
        unsigned int m_first_empty; //!< Index of the first empty predicate in a state
        unsigned int m_first_contains; //!< Index of the first contains function in a state
        unsigned int m_first_in; //!< Index of the first in function in a state
        unsigned int m_first_uses; //!< Index of the first uses function in a state
        unsigned int m_first_on_fire; //!< Index of the first onFire predicate in a state
        unsigned int m_first_rubble_cleared; //!< Index of the first rubbleCleared predicate in a state
        unsigned int m_first_repaired; //!< Index of the first repaired predicate in a state
        unsigned int m_first_healed; //!< Index of the first healed predicate in a state

        friend class SurvivorProblem;
    };
}
#endif //GRSTAPS_SURVIVOR_STATE_DECODER_HPP
