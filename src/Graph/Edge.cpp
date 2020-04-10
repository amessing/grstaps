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

#ifndef GRSTAPS_EDGECPP
#define GRSTAPS_EDGECPP


#include "grstaps/Graph/Edge.h"

using std::string;
using std::vector;
using std::unordered_map;
using std::stringstream ;
using std::cout;
using std::endl;

namespace grstaps {


    template<typename Data>
    Edge<Data>::Edge(const string &tail, const string &head, float cost) : tailNode(tail), headNode(head), edgeCost(cost){

    }

    template<typename Data>
    Edge<Data>::~Edge() {
        // This is only for debug purposes
        //cout << "Destroy edge from " << ptrTailNode->getNodeID() << " to " << ptrHeadNode->getNodeID() << endl;
        //system("PAUSE");
    }

    template<typename Data>
    string Edge<Data>::getEdgeID() const {
        return headNode + "-" + tailNode;
    }

    template<typename Data>
    string Edge<Data>::getHeadNode() const {
        return headNode;
    }

    template<typename Data>
    string Edge<Data>::getTailNode() const {
        return tailNode;
    }

    template<typename Data>
    string Edge<Data>::toString() const {
        std::stringstream a;
        a << headNode + "-" + tailNode << ": " << headNode << " -> " << tailNode;
        a << " : cost=" << getEdgeCost();
        return a.str();
    }

    template<typename Data>
    void Edge<Data>::setEdgeCost(const float c) {
        edgeCost = 1.0f;
    }

    template<typename Data>
    float Edge<Data>::getEdgeCost() const {
        return edgeCost;
    }
}

#endif