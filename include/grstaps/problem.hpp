#if 0
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

// local
#include "grstaps/task_planning/utils.hpp"
#include "grstaps/action.hpp"
#include "grstaps/location.hpp"
#include "grstaps/robot.hpp"

namespace grstaps
{
    /**
     * Creates a HMA-CTAMP problem
     *
     * todo: type trait magic
     */
    template <typename StateDecoder>
    class Problem
    {
    protected:
        using StateDecoderType = StateDecoder;
    public:
        /**
         * Default Constructor
         */
        Problem() = default;

        /**
         * Initializes this problem
         *
         * \param config The configuration for generating a problem
         */
        void init(const nlohmann::json& config) = 0;

        /**
         * Writes the problem config
         */
        virtual void write(const std::string& filepath) const = 0;

        /**
         * \returns The action specified by the \p id
         */
        const Action& action(unsigned int id) const
        {
            return m_actions[id];
        }

        /**
         * \returns The location specified by the \p id
         */
        const Location& location(unsigned int id) const
        {
            return m_locations[id];
        }

        /**
         * \returns The robot specified by the \p id
         */
        const Robot& robot(unsigned int id) const
        {
            return m_robots[id];
        }

        /**
         * \returns The initial state of the problem
         */
        const StateAssignment& initialState() const
        {
            return m_initial_state;
        }

        /**
         * \returns The goal of the problem
         */
        const StateAssignment& goal() const
        {
             return m_goal;
        }

        const StateDecoder& stateDecoder() const
        {
            return *m_state_decoder;
        }

    protected:
        std::vector<Action> m_actions; //!< A list of the actions that can be used to solve this problem
        std::vector<Location> m_locations; //!< A list of the locations that either robots or objects can be at
        std::vector<Robot> m_robots; //!< A list of the robots

        StateAssignment m_initial_state; //!< The initial state of the problem
        StateAssignment m_goal; //!< The state assignment representing the goal

        std::unique_ptr<StateDecoder> m_state_decoder;
    };
}

#endif //GRSTAPS_PROBLEM_HPP
#endif