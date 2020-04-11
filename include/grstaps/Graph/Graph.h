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

#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include "Node.h"
#include "Edge.h"
#include <grstaps/unordered_map/robin_hood.h>
//#include "tbb/concurrent_unordered_map.h"B

using std::string;
using std::vector;
//using std::unordered_map;
//using robin_hood::unordered_map;

namespace grstaps {

    /**
     * Graph class
     *
     * \note The Graph class defines a graph data structure. It consists of both nodes (or vertices) and edges (links) between the nodes.
     * \note Internally there is a list of nodes, and list of edges, each stored in a  Unordered_map with shared_ptr's to each element.
     *
     *
     */
    template<class Data>
    class Graph {
    public:

        /**
         *
         * Finds a node that is in the graph and returns a pointer to it returns null pointer if not in graph
         *
         *
         * \param string id of the node you wish to get
         *
        */
        nodePtr<Data> findNode(const string&) const;

        /**
         *
         * Prints out the whole graph
         * it will iterator over all nodes, and print each one's edges
         *
        */
        void print() const;

        /**
         *
         * Prints out all nodes
         *
        */
        int printNodeList() const;

        /**
         *
         * Clears the search state of all of the nodes
         *
        */
        void clearSearchState();

        /**
         *
         * Adds a node to the graph
         *
         * \param shared pointer to the node you wish to add
         * \param id of the new node
         *
        */
        void addNode(nodePtr<Data>& nodeToAdd, string &id, bool quick =true);


        /**
         *
         * Adds a node to the graph
         *
         * \param shared pointer to the node you wish to add
         *
        */
        void addNode(nodePtr<Data>&, bool quick =true);

        /**
         *
         * Adds a node to the graph
         *
         * \param the id of the new node
         * \param the data that will be inside of the new node
         *
        */
        void addNode(const string &, Data, bool quick =true);

        /**
         *
         * Adds a node to the graph
         *
         * \paran the id of the node you wish to ass
         * \param the pointer to the data of the node
         * \param the cost of the node
         * \param the heuristic cost of the node
         *
        */
        void addNode(const string& , Data, float, float, int quick=true);

        /**
         *
         * Adds a edge to the graph
         *
         * \param shared pointer to the edge to add
         *
        */
        void addEdge(edgePtr<Data> edgeToAdd, bool quick=true);

        /**
         *
         * Adds a edge to the graph
         *
         * \param shared pointer to the edge to add
         *
        */
        void addEdge(const string &, const string &, float, bool quick=true);

        /**
         *
         * Adds a edge to the graph
         *
         * \param shared pointer to the edge to add
         *
        */
        void addEdge(const string &, const string &, float, nodePtr<Data>&, nodePtr<Data>&, bool quick=true);


        /**
         *
         * Removes a node from the graph
         *
         * \paran the id of the node you wish to remove
         *
        */
        bool removeNode(string);

        /**
         *
         * Removes a edge from the graph
         *
         * \paran the id of the edge you wish to remove
         *
        */
        bool removeEdge(string);


        /**
         *
         * checks is node is in graph
         *
         * \paran the id of the node you wish to check
         *
        */
        bool nodeExist(string);

        /**
         *
         * checks is edge is in graph
         *
         * \paran the id of the edge you wish to check
         *
        */
        bool edgeExist(string);
        //tbb::concurrent_unordered_map<typename Key, typename T, typename Hasher, typename Key_equality, typename Allocator>(const Allocator &a) {}
        robin_hood::unordered_map<string, edgePtr < Data> > edgeList; //!< Unordered_map to the graphs edges
        robin_hood::unordered_map<string, nodePtr < Data> > nodeList;  //!< Unordered_map to the graphs nodes
    private:

    };

} //namespace grstaps

#endif /* GRAPH_H */

#ifndef GRAPH_CPP

#include "../src/Graph/Graph.cpp"

#endif