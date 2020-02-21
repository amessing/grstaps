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
#ifndef GRSTAPS_BASE_OBJECT_HPP
#define GRSTAPS_BASE_OBJECT_HPP

// global
#include <string>

// external
#include <nlohmann/json.hpp>

// local
#include "grstaps/noncopyable.hpp"

namespace grstaps
{
    /**
     * Container for non-controllable object
     */
    class Object : public Noncopyable
    {
       public:
        /**
         * Constructor
         *
         * \param type The type of the object
         * \param properties The properties of this object
         */
        Object(const std::string& type, const nlohmann::json& properties);

        /**
         * \returns The identifier for this object
         */
        unsigned int id() const;

        /**
         * \returns The type of this object
         */
        const std::string& type() const;

        /**
         * \returns Whether this object can be moved
         *
         * \note Is a wrapper for the Object#property function below
         */
        bool movable() const;

        /**
         * \returns The weight of this object
         *
         * \note Is a wrapper for the Object#property function below
         */
        float weight() const;

        /**
         * \returns Whether this object has the property specified by \p name
         */
        bool hasProperty(const std::string& name) const;

        /**
         * \param name The name of the property (e.g. movable)
         *
         * \tparam The type of the property
         *
         * \returns The property specified by \p name
         */
        template <typename PropertyType>
        const PropertyType& property(const std::string& name) const;

        /**
         * \returns The properties of this object
         */
        const nlohmann::json& properties() const;

       private:
        unsigned int m_id;            //!< Identifier for this specific object
        std::string m_type;           //!< Type of this object (e.g. box)
        nlohmann::json m_properties;  //!< A "magic" dictionary of the properties of this object

        static unsigned int s_next_object_id;  //!< The identifier for the next object to be created
    };
}  // namespace grstaps

#endif  // GRSTAPS_BASE_OBJECT_HPP
