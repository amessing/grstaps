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
        std::array<unsigned int, 11> m_location_type_start_indicies;
        std::array<unsigned int, 11> m_function_type_start_indicies; //!< Contains the starting index for each
        //!< type of function/predicate in a survivor state

        // Indicies in 'm_location_type_start_indicies'
        const unsigned int k_medicine_start = 0;
        const unsigned int k_small_crate_start = 1;
        const unsigned int k_large_crate_start = 2;
        const unsigned int k_water_start = 3;
        const unsigned int k_construction_kit_start = 4;
        const unsigned int k_pickup = 5;
        const unsigned int k_hospital = 6;
        const unsigned int k_survivor = 7;
        const unsigned int k_fire = 8;
        const unsigned int k_damaged_building = 9;
        const unsigned int k_total_locations = 10;

        // Indicies in 'm_function_type_start_indicies'
        const unsigned int k_location = 0;
        const unsigned int k_contains = 1;
        const unsigned int k_in = 2;
        const unsigned int k_uses = 3;
        const unsigned int k_used = 4; // Maybe should change one of these?
        const unsigned int k_healed = 5;
        const unsigned int k_on_fire = 6;
        const unsigned int k_rubble_cleared = 7;
        const unsigned int k_repaired = 8;
        const unsigned int k_mutex = 9;
        const unsigned int k_total = 10;

        friend class SurvivorProblem;
    };
}
#endif //GRSTAPS_SURVIVOR_STATE_DECODER_HPP
