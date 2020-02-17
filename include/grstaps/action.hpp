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
         */
        Action(const std::string& prototype,
               /* Preconditions */
               /* Effects */
               /* Durations */
               /* Traits */);

        /**
         * \returns The identifier for this action
         */
        unsigned int id() const;

        /**
         * \returns Preconditions
         */

        /**
         * \returns Effects
         */

        /**
         * \returns minimum and maximum duration
         */

        /**
         * \returns function for calculating duration
         */

        /**
         * \returns The required traits for this action
         */

       private:
        unsigned int m_id;  //!< The id of this specific action
        // condition (equation) - pointer/object/noncopyable
        // duration min, max (equation?) - pointer/object/noncopyable
        // effect (equation) - pointer/object/noncopyable
        // traits/trait mapping

        static unsigned int s_next_action_id;  //!< The id for the next action that is created
    };
}  // namespace grstaps

#endif  // GRSTAPS_BASE_ACTION_HPP
