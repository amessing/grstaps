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
#ifndef GRSTAPS_LOCATION_HPP
#define GRSTAPS_LOCATION_HPP

// global
#include <string>
#include <utility>

// external
#include <nlohmann/json.hpp>

// local
#include "grstaps/noncopyable.hpp"

namespace grstaps
{
    /**
     * Container for a location
     */
    class Location
    {
       public:
        /*
         * Default constructor
         *
         * /note for json
         */
        Location() = default;
        /**
         * Constructor
         *
         * \param type The type of this location
         * \param x The x component of this location
         * \param y The y component of this location
         */
        Location(const std::string& type, float x, float y);

        /**
         * \returns The id of this location
         */
        unsigned int id() const;

        /**
         * \returns The type of this location
         */
        const std::string& type() const;

        /**
         * \returns The x component of this location
         */
        float x() const;

        /**
         * \returns The y component of this location
         */
        float y() const;

        /**
         * \returns The coordinates of this location
         */
        std::pair<float, float> coordinates() const;

       protected:
        unsigned int m_id;   //!< The identifier for this location
        std::string m_type;  //!< The type of this location (e.g. hospital)
        float m_x;           //!< The x component of this location
        float m_y;           //!< The y component of this location

        static unsigned int s_next_location_id;  //!< The identifier for this next location to be created

        friend void to_json(nlohmann::json& j, const Location& l);
        friend void from_json(const nlohmann::json& j, Location& l);
    };

    void to_json(nlohmann::json& j, const Location& l);
    void from_json(const nlohmann::json& j, Location& l);
}  // namespace grstaps
#endif  // GRSTAPS_LOCATION_HPP