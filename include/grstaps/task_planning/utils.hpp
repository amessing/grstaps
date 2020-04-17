/*
 * Copyright (C) 2020 Andrew Messing
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

#ifndef GRSTAPS_UTILS_HPP
#define GRSTAPS_UTILS_HPP

// global
#include <cstdint>
#include <utility>
#include <vector>

namespace grstaps
{
    using Step = uint16_t;
    using TimePoint = uint16_t;
    using OrderingConstraint = std::pair<Step, Step>;

    using StateVariable = uint16_t;
    using StateVariableValue = uint16_t;
    using Fluent = std::pair<StateVariable, StateVariableValue>;
    using StateAssignment = std::vector<Fluent>;

    inline StateVariable variable(Fluent f)
    {
        return f.first;
    }

    inline StateVariableValue value(Fluent f)
    {
        return f.second;
    }
}

#endif //GRSTAPS_UTILS_HPP
