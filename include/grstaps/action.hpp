/*
 * Copyright (C)2020 Andrew Messing
 *
 * GRSTAPS is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 3 of the License,
 * or any later version.
 *
 * GRSTAPS is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GRSTAPS; if not, write to the Free Software Foundation,
 * Inc., #59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#ifndef GRSTAPS_BASE_ACTION_HPP
#define GRSTAPS_BASE_ACTION_HPP

// global
#include <vector>

// local
#include "grstaps/noncopyable.hpp"

namespace grstaps
{
    /**
     * Container for all aspects of an action that any of the subsystems need
     */
    class Action : public Noncopyable
    {
       public:
        /**
         * Constructor
         *
         * \param starting_location The identifier for the location this action is started at
         */
        Action(unsigned int starting_location);

        /**
         * \returns The identifier for this action
         */
        unsigned int id() const;

#if 0  // Ignoring for now
        /**
         * \returns The condiitions for this action to start
         */
        const std::vector<Fluent>& startConditions() const;

        /**
         * \returns The conditions that need to hold while this action is happening
         */
        const std::vector<Fluent>& overAllConditions() const;

        /**
         * \returns The conditions for this action to end
         */
        const std::vector<Fluent>& endConditions() const;

        /**
         * \returns The effects that happen at the start of this action
         */
        const std::vector<Fluent>& startEffects() const;

        /**
         * \returns The effects that happen at the end of this action
         */
        const std::vector<Fluent>& endEffects() const;

        /**
         * \returns The interval of the duration of this action
         */
        const std::pair<Duration, Duration>& durationRange() const;

        /**
         * \returns The minimum duration of this action
         */
        const Duration& minimumDuration() const;

        /**
         * \returns The maximum duration of this action
         */
        const Duration& maximumDuration() const;
#endif

        /**
         * \returns The required traits for this action
         */

        /**
         * \returns The id of the location that this action starts at
         */
        unsigned int startingLocation() const;

       private:
        unsigned int m_id;                 //!< The id of this specific action
        unsigned int m_starting_location;  //!< The id of the location that this action starts at

#if 0  // Ignoring for now
        std::vector<Fluent>
            m_start_conditions;  //!< A conjunction of the conditions that need to hold at the start of this action
        std::vector<Fluent> m_over_all_conditions;  //!< A conjunction of the conditions that need to hold while this
                                                    //!< action is happening
        std::vector<Fluent>
            m_end_conditions;  //!< A conjunction of the conditions that need to hold when this action finishes
        std::pair<Duration, Duration> m_durations;  //!< A pair of the minimum and maximum durations
        std::vector<Fluent> m_start_effects;  //!< A conjunction of the effects that happen at the start of this action
        std::vector<Fluent> m_end_effects;    //!< A conjunction of the effects that happen at the end of this action
#endif
        // traits/trait mapping

        static unsigned int s_next_action_id;  //!< The id for the next action that is created
    };
}  // namespace grstaps

#endif  // GRSTAPS_BASE_ACTION_HPP
