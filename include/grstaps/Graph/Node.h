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

#ifndef GRSTAPS_NODE
#define GRSTAPS_NODE

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "grstaps/Graph/Edge.h"
#include <../lib/unordered_map/robin_hood.h>


// Explicitly call out types we are using, instead of "using namespace"
// due to conflicts between boost and std smart pointer types
using std::string;
using std::vector;

namespace grstaps
{
    // Forward Declarations
    template <typename Data>
    class Edge;

    template <typename Data>
    using edgePtr = typename std::shared_ptr<Edge<Data>>;

    template <typename Data>
    using edgeWeakPtr = typename std::weak_ptr<Edge<Data>>;

    template <typename Data>
    using nodePtr = typename std::shared_ptr<Node<Data>>;

    template <typename Data>
    class Node  // this file
    {
       public:
        /**
         * Constructor
         *
         * \param the string id of the new node
         * \param the node data
         *
         */
        Node(string, Data&);

        /**
         * Constructor
         *
         * \param the string id of the new node
         * \param the node data
         *
         */
        Node(Node&);

        enum ExploredStatus
        {
            UNEXPLORED,
            FRONTIER,
            EXPLORED
        }; /*!< Describes the current state of node in search process */

        /**
         * Getter for node id
         *
         * \returns The nodes id as a string
         */
        string getNodeID() const;

        /**cout << nodeToAdd->getID();
         *
         * Setter Current Path cost to node
         *
         * \param What float to set the nodes path cost too
         *
         */
        void setPathCost(const float);

        /**
         *
         * Getter for node search status
         *
         * \returns The nodes id as a float
         */
        float getPathCost() const;

        /**
         *
         * Setter Node Heuristic
         *
         * \param What float to set the nodes heuristic cost too
         *
         */
        void setHeuristic(const float);

        /**
         *
         * Getter for node heuristic cost
         *
         * \returns The nodes heuristic cost as a float
         */
        float getHeuristic() const;

        /**
         *
         * Setter Node data pointer
         *
         * \param pointer to the nodes data
         *
         */
        void setData(const Data&);

        /**
         *
         * Getter for node data
         *
         * \returns A pointer to the nodes data
         */
        Data& getData();

        /**
         *
         * Prints all of a nodes parents
         *
         */
        void printParents() const;

        /**
         *
         * Prints all of a nodes children
         *
         */
        void printChildren() const;

        /**
         *
         * Clears a nodes status, heuristic cost, and path cost
         *
         */
        void clearSearchState();

        /**
         *
         * sets the nodes status, path cost and add new parent edge
         *
         * \param nodes status
         * \param float containing the path cost of the node
         * \param pointer to edge to connect
         *
         * \returns The nodes id as a float
         */
        void setSearchState(float, edgePtr<Data>);

        /**
         *
         * returns a pointer to the parent node specified by the passed id
         *
         * \param parent node id
         *
         * \returns a pointer to the parent node
         */
        std::weak_ptr<Node>& getParentNode(const std::string) const;

        /**
         *
         * Removes a parent node
         *
         * \param id of the parent node to remove
         *
         */
        bool removeParentNode(const std::string);

        /**
         *
         * returns a pointer to the child node specified by the passed id
         *
         * \param child node id
         *
         * \returns a pointer to the child node
         */
        std::weak_ptr<Node>& getChildNode(const std::string) const;

        /**
         *
         * Adds a new child node
         *
         * \param pointer to the edge between the new child and this node
         *
         */
        void addChildEdge(edgePtr<Data>, bool quick = true);

        /**
         *
         * Removes a child node
         *
         * \param id of the child node to remove
         *
         */
        bool removeChildNode(const std::string);

        /**
         *
         * Adds a new child node
         *
         * \param pointer to the new child node
         * \param the cost of the edge between the new child and this node
         *
         */
        void addChildNode(nodePtr<Data>, const float);

        /**
         *
         * Adds a new parent node
         *
         * \param pointer to the edge between the new parent and this node
         *
         */
        void addParentEdge(edgePtr<Data>, bool quick = true);

        /**
         *
         * Adds a new parent node
         *
         * \param pointer to the new parent node
         * \param the cost of the edge between the new parent and this node
         *
         */
        void addParentNode(nodePtr<Data>, const float);

        /**
         *
         * Adds a new parent node
         *
         * \param pointer to the edge between the new parent and this node
         *
         */
        void addParentEdge(edgePtr<Data>, const string&, bool quick = true);

        /**
         *
         * Adds a new child node
         *
         * \param pointer to the edge between the new child and this node
         *
         */
        void addChildEdge(edgePtr<Data>, const string&, bool quick = true);

        /**
         *
         * Set node id
         *
         * \param new node id
         *
         */
        void setID(string);

        robin_hood::unordered_map<string, edgePtr<Data>> enteringEdges;  //!< Unordered_map to the parent nodes */
        robin_hood::unordered_map<string, edgePtr<Data>> leavingEdges;   //!< Unordered_map to the child  nodes*/

       private:
        string nodeID;      //!< Whether the map has been set for the motion planner
        float pathCost;   //!< Float containing the nodes heuristic cost g()
        float heuristic;  //!< Float containing the nodes heuristic cost h()
        Data nodeData;      //!< Pointer to the nodes data
    };

    /**Node
     *
     *\note The NodeCompareCost defines function objects that are passed
     *\note to a priority queue or similar data structure for correct ordering
     *\note of Node objects. This implementation sets the shortest path to
     *\note the Node popped first.
     *
     */
    template <typename Data>
    class NodeCompareCost
    {
       public:
        bool operator()(const nodePtr<Data>& n1, const nodePtr<Data>& n2) const
        {
            return n1->getPathCost() > n2->getPathCost();
        }
    };

    /**
     *
     *\note The NodeCompareHeuristic defines function objects that are passed
     *\note to a priority queue or similar data structure for correct ordering
     *\note of Node objects. This implementation sets the shortest path to
     *\note the Node popped first.
     *
     */
    template <typename Data>
    class NodeCompareHeuristic
    {
       public:
        bool operator()(const nodePtr<Data>& n1, const nodePtr<Data>& n2) const
        {
            return n1->getHeuristic() > n2->getHeuristic();
        }
    };

    /**
     *
     *\note The NodeCompareHeuristic defines function objects that are passed
     *\note to a priority queue or similar data structure for correct ordering
     *\note of Node objects. This implementation sets the shortest path to
     *\note the Node popped first.
     *
     */
    template <typename Data>
    class NodeCompareF
    {
       public:
        bool operator()(const nodePtr<Data>& n1, const nodePtr<Data>& n2) const
        {
            float f1 = n1->getHeuristic() + n1->getPathCost();
            float f2 = n2->getHeuristic() + n1->getPathCost();
            return f1 > f2;
        }
    };

}  // namespace grstaps

#endif  // GRSTAPS_NODES

#ifndef GRSTAPS_NODECPP
#include "../src/Graph/Node.cpp"
#endif