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
#ifndef GRSTAPS_KNOWLEDGE_HPP
#define GRSTAPS_KNOWLEDGE_HPP

// global
#include <vector>

// local
#include "grstaps/noncopyable.hpp"

namespace grstaps
{
    // Forward declarations
    class Action;
    class Location;
    class Object;
    class Robot;

    /**
     * Centralized container for domain knowledge
     */
    class Knowledge : public Noncopyable
    {
       public:
        /**
         * \returns A singleton to the knowledge container
         */
        static Knowledge& instance();

        /**
         * \returns The action at \p id
         */
        const Action& action(unsigned int id) const;

        /**
         * \returns The object at \p id
         */
        const Object& object(unsigned int id) const;

        /**
         * \returns The location at \p id
         */
        const Location& location(unsigned int id) const;

        /**
         * \returns The robot at \p id
         */
        const Robot& robot(unsigned int id) const;

       private:
        Knowledge() = default;

        std::vector<Action> m_actions;
        std::vector<Object> m_objects;
        std::vector<Location> m_locations;
        std::vector<Robot> m_robots;
    };
}  // namespace grstaps

#endif  // GRSTAPS_KNOWLEDGE_HPP