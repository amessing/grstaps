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
// external
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

// local
#include <grstaps/constants.hpp>
#include <grstaps/survivor_problem.hpp>

namespace grstaps
{
    namespace test
    {
        TEST(Problem, survivor)
        {
            nlohmann::json config =
                {
                    {constants::k_min_num_hospitals, 1},
                    {constants::k_max_num_hospitals, 3},
                    {constants::k_min_num_survivors, 3},
                    {constants::k_max_num_survivors, 10},
                    {constants::k_min_num_fire, 3},
                    {constants::k_max_num_fire, 10},
                    {constants::k_min_num_damaged_buildings, 3},
                    {constants::k_max_num_damaged_buildings, 10},
                    {constants::k_extra_medicine, 3},
                    {constants::k_extra_small_crates, 3},
                    {constants::k_extra_large_crates, 3},
                    {constants::k_extra_small_water_containers, 3},
                    {constants::k_extra_large_water_containers, 3},
                };

            SurvivorProblem problem;
            problem.init(config);
        }
    }
}
#endif