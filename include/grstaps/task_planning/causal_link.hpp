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
#ifndef GRSTAPS_CAUSAL_LINK_HPP
#define GRSTAPS_CAUSAL_LINK_HPP

// local
#include "grstaps/task_planning/utils.hpp"

namespace grstaps
{
    /**
     * Container for a causal link
     */
    class CausalLink
    {
    public:
        CausalLink(const OrderingConstraint& ordering,
            const Fluent& fluent);

        TimePoint first() const;
        TimePoint second() const;
        StateVariable variable() const;
        StateVariableValue value() const;

    private:
        OrderingConstraint m_ordering;
        Fluent m_fluent;
    };
}

#endif //GRSTAPS_CAUSAL_LINK_HPP
