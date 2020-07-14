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
#ifndef GRSTAPS_NONCOPYABLE_HPP
#define GRSTAPS_NONCOPYABLE_HPP

namespace grstaps
{
    /**
     * Class to be used as a base class for non-copyable classes
     */
    class Noncopyable
    {
       public:
        /**
         * Default Constructor
         */
        Noncopyable()  = default;

        /**
         * Move Constructor
         */
        Noncopyable(Noncopyable&&) = default;

        /**
         * Destructor
         */
        ~Noncopyable() = default;

        /**
         * Move assignment operator
         */
        Noncopyable& operator=(Noncopyable&&) = default;

       private:
        Noncopyable(const Noncopyable&) = delete;
        Noncopyable& operator=(const Noncopyable&) = delete;
    };
}  // namespace grstaps

#endif  // GRSTAPS_NONCOPYABLE_HPP
