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

#ifndef SEARCH_BASE_H
#define SEARCH_BASE_H

//internal
#include "grstaps/Graph/Node.h"
#include "grstaps/Graph/Graph.h"
#include "grstaps/Search/GoalLocator.h"
#include "grstaps/Search/NodeExpander.h"
#include "grstaps/Search/SearchResultPackager.h"

//external
#include <boost/heap/binomial_heap.hpp>


namespace grstaps {

    template<class Data>
    class SearchBase {
    public:
        /**
        * Constructor
        *initialNodePtr->setStatus(Node::FRONTIER);
        * \param the graph that the search is through
        *
        */
        SearchBase(Graph<Data> &,  nodePtr<Data>&);

        /**
        * Default Constructor
        *
        */
        SearchBase();

        /**
         * Search through the graph and find a node
         *
         * \param pointer to the starting node
         * \param functor that returns a bool to say if the current node is a goal
         * \param node expander that is a functor that expands a past node adding its children to the graph
         * \param a functor that formats the solution found
         * \param a comparator functor that is used to order the nodes on the frontier
         *
         * \returns a search results object that contains all needed return information from the search
         */
        virtual void search(GoalLocator<Data>*, NodeExpander<Data>*, SearchResultPackager<Data>*) = 0;




    protected:
        nodePtr<Data> initialNodePtr; //!< Pointer to Starting Node in the search
        Graph<Data> graph;
    };

} // namespace GRSTAPS


#endif //SEARCH_BASE_H

#ifndef SEARCH_BASE_CPP
#include "../src/Search/SearchBase.cpp"
#endif