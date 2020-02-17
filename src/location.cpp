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
#include "grstaps/location.hpp"

namespace grstaps
{
    unsigned int Location::s_next_location_id = 0;

    Location::Location(const std::string& type, float x, float y)
        : m_id(s_next_location_id++)
        , m_type(type)
        , m_x(x)
        , m_y(y)
    {}

    unsigned int Location::id() const
    {
        return m_id;
    }

    const std::string& Location::type() const
    {
        return m_type;
    }

    float Location::x() const
    {
        return m_x;
    }

    float Location::y() const
    {
        return m_y;
    }

    std::pair<float, float> Location::coordinates() const
    {
        return std::make_pair(m_x, m_y);
    }

}  // namespace grstaps