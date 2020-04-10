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

#ifndef GRSTAPS_GOALLOCATOR
#define GRSTAPS_GOALLOCATOR


#include "grstaps/Graph/Node.h"
#include "grstaps/Graph/Edge.h"
#include "grstaps/Graph/Graph.h"


namespace grstaps {

    template<typename Data>
    using nodePtr = typename boost::shared_ptr<Node<Data>>;

    /**
     * Functor for finding a goal in a search problem
     *
     * \note need to implement a verision of this for our search problems
     *
     */
    template<class Data>
    class GoalLocator {
        public:
            // This operator overloading enables calling
            // operator function () on objects of increment
            virtual bool operator()(Graph<Data>&, nodePtr<Data>&);
    };

} // namespace grstaps
#endif //GRSTAPS_GOALLOCATOR

#ifndef GRSTAPS_GOALLOCATORCPP
#include "../src/Search/GoalLocator.cpp"
#endif
