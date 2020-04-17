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

#ifndef GRSTAPS_CONSTANTS_HPP
#define GRSTAPS_CONSTANTS_HPP

namespace grstaps
{
    namespace constants
    {
        // Problem Config Keywords
        const char * const k_min_num_hospitals = "min_num_hospitals";
        const char * const k_max_num_hospitals = "max_num_hospitals";
        const char * const k_min_num_survivors = "min_num_survivors";
        const char * const k_max_num_survivors = "max_num_survivors";
        const char * const k_min_num_fire = "min_num_fire";
        const char * const k_max_num_fire = "max_num_fire";
        const char * const k_min_num_damaged_buildings = "min_num_damaged_buildings";
        const char * const k_max_num_damaged_buildings = "max_num_damaged_buildings";
        const char * const k_extra_medicine = "extra_medicine";
        const char * const k_extra_small_crates = "extra_small_crates";
        const char * const k_extra_large_crates = "extra_large_crates";
        const char * const k_extra_small_water_containers = "extra_small_water_containers";
        const char * const k_extra_large_water_containers = "extra_large_water_containers";

        // Location Names
        const char * const k_medicine_location = "medicine_start";
        const char * const k_water_location = "water_start";
        const char * const k_small_crate_location = "small_crate_packaging";
        const char * const k_large_crate_location = "large_crate_packaging";
        const char * const k_construction_kit_location = "construction_kit_start";
        const char * const k_pickup_location = "pickup";
        const char * const k_hospital_location = "hospital";
        const char * const k_survivor_location = "survivor";
        const char * const k_fire_location = "fire";
        const char * const k_damaged_building_location = "damaged_building";
    }
}

#endif //GRSTAPS_CONSTANTS_HPP
