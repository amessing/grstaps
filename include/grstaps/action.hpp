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
#ifndef GRSTAPS_ACTION_HPP
#define GRSTAPS_ACTION_HPP

// global
#include <vector>

// local
#include "grstaps/noncopyable.hpp"
#include "grstaps/task_planning/utils.hpp"

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

        /**
         * \returns The duration of this action
         */
        float duration() const;

        /**
         * \returns The conditions for this action to start
         */
        const StateAssignment& startConditions() const;

        /**
         * \returns The conditions that need to hold while this action is happening
         */
        const StateAssignment& overAllConditions() const;

        /**
         * \returns The conditions for this action to end
         */
        const StateAssignment& endConditions() const;

        /**
         * \returns The effects that happen at the start of this action
         */
        const StateAssignment& startEffects() const;

        /**
         * \returns The effects that happen at the end of this action
         */
        const StateAssignment& endEffects() const;

        /**
         * \returns Whether this action is the fictitious action for the goal
         */
        bool isGoal() const;

        /**
         * \returns The required traits for this action
         */
         const std::vector<float>& requirements() const;

        /**
         * \returns The id of the location that this action starts at
         */
        unsigned int startingLocation() const;

       private:
        unsigned int m_id;                 //!< The id of this specific action

        // For the motion planner
        unsigned int m_starting_location;  //!< The id of the location that this action starts at

        // For the scheduler
        float m_duration; //!< The duration of this action

        // For the task planner
        StateAssignment m_start_conditions; //!< A conjunction of the conditions that need to hold at the start of this action
        StateAssignment m_over_all_conditions; //!< A conjunction of the conditions that need to hold while this action is happening
        StateAssignment m_end_conditions; //!< A conjunction of the conditions that need to hold at the end of this action

        StateAssignment m_start_effects; //!< A conjunction of the effects that happen at the start of this action
        StateAssignment m_end_effects; //!< A conjunction of the effects that happen at the end of this action

        bool m_is_goal; //!< Whether this action is the fictitious action for the goal

        // For the task allocator
        std::vector<float> m_requirements;

        static unsigned int s_next_action_id;  //!< The id for the next action that is created

        // Allows the Problem class to easily build an action
        //friend class Problem;
    };
}  // namespace grstaps

#endif  // GRSTAPS_ACTION_HPP
