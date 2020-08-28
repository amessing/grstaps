/*
 * Copyright (C)2020 Glen Neville
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

#ifndef GRSTAPS_SEARCHCOMPARATORS
#define GRSTAPS_SEARCHCOMPARATORS

#include "grstaps/Graph/Graph.h"

namespace grstaps
{
    template <typename Data>
    using nodePtr = typename boost::shared_ptr<Node<Data>>;

    /**
     * Functor comparing nodes
     *
     * \note used for the heap
     *
     */
    template <typename Data>
    class SearchComparators
    {
       public:
        // This operator overloading enables calling
        // operator function () on objects of increment
        virtual bool operator()(nodePtr<Data>, nodePtr<Data>) const = 0;
    };

}  // namespace grstaps

#endif  // GRSTAPS_SEARCHCOMPARATORS