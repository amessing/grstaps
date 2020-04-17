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

#ifndef GRSTAPS_SURVIVOR_PROBLEM_HPP
#define GRSTAPS_SURVIVOR_PROBLEM_HPP

// global
#include <random>

// local
#include "grstaps/problem.hpp"
#include "grstaps/survivor_state_decoder.hpp"

namespace grstaps
{
    /**
     * A specific MA-CTAMP problem that represents the aftermath of a natural disaster
     */
    class SurvivorProblem : public Problem<SurvivorStateDecoder>
    {
        using Coordinates = std::pair<unsigned int, unsigned int>;
    public:
        /**
         * Default constructor
         */
        SurvivorProblem() = default;

        /**
         * Initializes the problem
         *
         * \param config The configuration
         * \param state_decoder Utility for decoding a state
         */
        virtual void init(const nlohmann::json& config, SurvivorStateDecoder& state_decoder) override ;

    private:
        /**
         * Builds the map for the motion planner
         */
        void buildMap(const nlohmann::json& config);

        /**
         * Creates all the important locations
         *
         * \param config Configuration for the problem
         * \param vars Important information to be passed between internal functions
         */
        void createLocations(const nlohmann::json& config, SurvivorStateDecoder& state_decoder);

        /**
         * Creates the initial state
         *
         * \param config Configuration for the problem
         * \param vars Important information to be passed between internal functions
         */
        void createInitialState(const nlohmann::json& config, SurvivorStateDecoder& state_decoder);

        /**
         * Creates the goal
         *
         * \param config Configuration for the problem
         * \param vars Important information to be passed between internal functions
         */
        void createGoal(const nlohmann::json& config, SurvivorStateDecoder& state_decoder);

        /**
         * Creates the total list of grounded actions
         *
         * \param config Configuration for the problem
         * \param vars Important information to be passed between internal functions
         */
        void createActions(const nlohmann::json& config, SurvivorStateDecoder& state_decoder);

        /**
         * Generates a random number
         */
        static unsigned int randomUnsignedInt(unsigned int min, unsigned int max);

        /**
         * Generates a random float
         */
        static float randomFloat(float min, float max);

        /**
         * Generates a random float between 0 and 1
         */
        static float randomFloat();

        /**
         * Creates an (x, y) location outside of the warehouse
         */
        Coordinates createLocationOutsideWarehouse();
        Coordinates createMedicineStartLocation();
        Coordinates createWaterStartLocation();
        Coordinates createSmallCrateStartLocation();
        Coordinates createLargeCrateStartLocation();
        Coordinates createConstructionKitStartLocation();
        Coordinates createPickupLocation();

        static std::unique_ptr<std::random_device> s_rd;
        static std::unique_ptr<std::mt19937> s_generator;
    };
}

#endif //GRSTAPS_SURVIVOR_PROBLEM_HPP
