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
#include "grstaps/action.hpp"

namespace grstaps
{
    unsigned int Action::s_next_action_id = 0;

    Action::Action(const std::string& prototype,
                   /* Preconditions */
                   /* Effects */
                   /* Durations */
                   /* Traits */)
        : m_id(s_next_action_id++)
        , m_prototype(prototype)
    {}

    unsigned int Action::id() const
    {
        return m_id;
    }
}  // namespace grstaps