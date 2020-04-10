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

#ifndef GRSTAPS_SEARCHRESULTSPACKAGER
#define GRSTAPS_SEARCHRESULTSPACKAGER

#include "grstaps/Graph/Graph.h"


namespace grstaps {

    template<typename Data>
    using nodePtr = typename boost::shared_ptr<Node<Data>>;

    /**
     * Functor for taking in a goal node and returning the answer in an appropriate form
     *
     * \note need to implement a verision of this for our search problems
     *
     */
    template<typename Data>
    class SearchResultPackager{
    public:
        void addResults(Graph<Data>& resultGraph, nodePtr<Data>& goalNode, bool goalLocated);

        Graph<Data>* graph;
        nodePtr<Data> finalNode;
        bool foundGoal;
    };

} // namespace grstaps
//#include "../src/Search/SearchResultPackager.cpp"
#endif //GRSTAPS_SEARCHRESULTSPACKAGER

#ifndef GRSTAPS_SEARCHRESULTSPACKAGERCPP
#include "../src/Search/SearchResultPackager.cpp"
#endif