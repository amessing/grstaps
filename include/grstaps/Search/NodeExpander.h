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

#ifndef GRSTAPS_NODEEXPANDER
#define GRSTAPS_NODEEXPANDER

#include "grstaps/Graph/Graph.h"
#include "grstaps/Graph/Node.h"
#include "grstaps/Search/Cost.h"
#include "grstaps/Search/Heuristic.h"

namespace grstaps
{
    template <typename Data>
    using nodePtr = typename boost::shared_ptr<Node<Data>>;

    /**
     * Functor for expanding the graph by adding children to a node
     *
     * \note need to implement a verision of this for our search problems
     *
     */
    template <typename Data>
    class NodeExpander
    {
       public:
        /**
         * Constructor
         *
         * \param heuristic objectconst
         * \param cost object
         *
         */
        NodeExpander(boost::shared_ptr<const Heuristic>, boost::shared_ptr<const Cost>);

        /**
         * Constructor
         *
         * \param heuristic object
         * \param cost object
         *
         */
        NodeExpander();

        // This operator overloading enables calling
        // operator function () on objects of increment
        virtual bool operator()(Graph<Data>&, nodePtr<Data>) const = 0;

        boost::shared_ptr<const Heuristic> heuristicFunc;  //!< heuristic object
        boost::shared_ptr<const Cost> costFunc;            //!< cost object
    };

}  // namespace grstaps

#endif  // GRSTAPS_NODEEXPANDER

#ifndef GRSTAPS_NODEEXPANDERCPP
#include "../src/Search/NodeExpander.cpp"
#endif