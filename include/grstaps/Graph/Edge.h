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

#ifndef GRSTAPS_EDGE
#define GRSTAPS_EDGE

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

// Explicitly call out types we are using, instead of "using namespace"
// due to conflicts between boost and std smart pointer types
using std::string;
using std::vector;

namespace grstaps
{
    // Forward declaration of class Node, as well as typedefs
    // An Edge is like an arrow, and contains a pointer to the
    // Node object at its head and tail bothint, as shared pointers.
    template <class Data>
    class Node;

    template <typename Data>
    using nodePtr = typename std::shared_ptr<Node<Data>>;

    /**
     * Edge Class for graph library.
     *
     * \note Edges are data agnostic.
     *
     */
    template <typename Data>
    class Edge
    {
       public:
        /**
         *
         * Constructor
         *
         * \param Head Node
         * \param Tail NodeGetter for edge tail node
         * \param Edge Cost
         *
         */
        Edge(const string &, const string &, float);

        /**
         *
         * Destructor
         *
         */
        ~Edge();

        /**
         *
         * Getter for edge id
         *
         * \return returns the string that is the edge id
         *
         */
        string getEdgeID() const;

        /**
         *
         * Getter for edge cost
         *
         * \return returns the float that is the edges cost
         *
         */
        float getEdgeCost() const;

        /**
         *
         * Getter for the edges head node
         *
         * \return returns a pointer to the edges head node
         *
         */
        string getHeadNode() const;

        /**
         *
         * Getter for edge tail node
         *
         * \return returns a pointer to the edges head node
         *
         */
        string getTailNode() const;

        /**
         *
         * Converts a edge to a string
         *
         * \return returns a conversion of the edge to a string
         *
         */
        string toString() const;

        /**
         *
         * Setter for edge cost
         *
         * \param what to set the edge cost too
         *
         */
        void setEdgeCost(float);

       private:
        float edgeCost;   //!< The cost to travese the edge
        string tailNode;  //!< NodeID of tail node
        string headNode;  //!< NodeID of head node

        // private utility functions
        //
    };
}  // namespace grstaps

#endif /* GRSTAPS_EDGE */

#ifndef GRSTAPS_EDGECPP
#include "../src/Graph/Edge.cpp"
#endif
