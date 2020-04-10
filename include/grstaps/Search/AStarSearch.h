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

#ifndef GRSTAPS_ASTAR
#define GRSTAPS_ASTAR


//internal
#include "grstaps/Graph/Node.h"
#include "grstaps/Graph/Graph.h"
#include "grstaps/Search/GoalLocator.h"
#include "grstaps/Search/NodeExpander.h"
#include "grstaps/Search/SearchResultPackager.h"
#include "grstaps/Search/SearchComparators.h"
#include "SearchBase.h"


//external
#include <boost/heap/binomial_heap.hpp>


namespace grstaps {

    template<class Data>
    class AStarSearch : public SearchBase<Data> {
    public:

        /**
        * Constructor
        *
        * \param the graph that the search is through
        *
        */
        AStarSearch(Graph<Data> &, nodePtr<Data> &);

        /**
       * Copy Constructor
       *
       * \param the search problem you are going to copy from
       * \param the new expansion function you will use on the copy
       *
       */
        AStarSearch(AStarSearch<Data> &, NodeExpander<Data> &);

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
        virtual void search(GoalLocator<Data>*, NodeExpander<Data>*, SearchResultPackager<Data>*);

        /**
         * Update the Current node
         *
         */
        bool updateCurrent();

    private:
        nodePtr<Data> currentNode;
        boost::heap::binomial_heap< nodePtr<Data>, boost::heap::compare<NodeCompareF<Data>> > frontier;
        boost::heap::binomial_heap< nodePtr<Data>, boost::heap::compare<NodeCompareF<Data>> > closedList;

    };
}// namespace grstaps

//#include "../src/Search/AStarSearch.cpp"
#endif //GRSTAPS_ASTAR
