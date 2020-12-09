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

#ifndef GRSTAPS_NODECPP
#define GRSTAPS_NODECPP

#include "grstaps/Graph/Node.h"

namespace grstaps
{
    template <class Data>
    Node<Data>::Node(string id, Data& data)
        : nodeID(id)
        , pathCost(0.0f)
        , heuristic(0.0f)
        , nodeData(data)
    {}

    template <class Data>
    Node<Data>::Node(grstaps::Node<Data>& copyNode)
    {
        enteringEdges = copyNode.enteringEdges;
        leavingEdges  = copyNode.leavingEdges;
        nodeID        = copyNode.nodeID;
        pathCost      = copyNode.pathCost;
        heuristic     = copyNode.heuristic;
        nodeData      = copyNode.nodeData;
    }


    template <class Data>
    std::string Node<Data>::getNodeID() const
    {
        return nodeID;
    }

    template <class Data>
    void Node<Data>::setPathCost(const float c)
    {
        if(c >= 0.0f)
            pathCost = c;
        else
            pathCost = 0.0f;
    }

    template <class Data>
    float Node<Data>::getPathCost() const
    {
        return pathCost;
    }

    template <class Data>
    void Node<Data>::setHeuristic(const float h)
    {
        if(h >= 0.0f)
            heuristic = h;
        else
            heuristic = 0.0f;
    }

    template <class Data>
    float Node<Data>::getHeuristic() const
    {
        return heuristic;
    }

    template <class Data>
    void Node<Data>::setSearchState(float cost, edgePtr<Data> parent)
    {
        // convenience method to set multiple values in a single call
        setPathCost(cost);
        addParentEdge(parent);
    }

    template <class Data>
    void Node<Data>::setData(const Data& data)
    {
        nodeData = data;
    }

    template <class Data>
    Data& Node<Data>::getData()
    {
        return nodeData;
    }

    template <class Data>
    void Node<Data>::printParents() const
    {
        // iterate over leaving edges, and call each one's toString() method

        for(auto itr = enteringEdges.begin(); itr != enteringEdges.end(); ++itr)
        {
            // boost::shared_ptr<Edge> ptrSharedEdge(*itr);
            std::cout << "      " << itr->second->toString() << std::endl;
        }
    }

    template <class Data>
    void Node<Data>::printChildren() const
    {
        // iterate over leaving edges, and call each one's toString() method

        for(auto itr = leavingEdges.begin(); itr != leavingEdges.end(); ++itr)
        {
            std::cout << "      " << itr->second->toString() << std::endl;
        }
    }

    template <class Data>
    void Node<Data>::clearSearchState()
    {
        // set all values back to initial states
        setStatus(UNEXPLORED);
        setPathCost(0.0f);
        setHeuristic(0.0f);
    }

    template <class Data>
    std::weak_ptr<Node<Data>>& Node<Data>::getParentNode(const std::string id) const
    {
        auto foundNode           = enteringEdges.find(id);
        edgePtr<Data> parentNode = NULL;
        if(foundNode != enteringEdges.end())
        {
            parentNode = foundNode->second;
        }
        return parentNode->getHeadNode();
    }

    template <class Data>
    std::weak_ptr<Node<Data>>& Node<Data>::getChildNode(const std::string id) const
    {
        auto foundNode          = leavingEdges.find(id);
        edgePtr<Data> childNode = NULL;
        if(foundNode != leavingEdges.end())
        {
            childNode = foundNode->second;
        }

        return childNode->getTailNode();
    }

    template <class Data>
    void Node<Data>::addParentEdge(edgePtr<Data> parent, bool quick)
    {
        if(quick)
        {
            enteringEdges[parent->getEdgeID()] = parent;
        }
        else
        {
            if(enteringEdges.find(parent->getEdgeID()) == enteringEdges.end())
            {
                enteringEdges[parent->getEdgeID()] = parent;
            }
        }
    }

    template <class Data>
    void Node<Data>::addChildEdge(edgePtr<Data> child, bool quick)
    {
        if(quick)
        {
            leavingEdges[child->getEdgeID()] = child;
        }
        else
        {
            if(leavingEdges.find(child->getEdgeID()) == leavingEdges.end())
            {
                leavingEdges[child->getEdgeID()] = child;
            }
        }
    }

    template <class Data>
    void Node<Data>::addParentEdge(edgePtr<Data> parent, const string& id, bool quick)
    {
        if(quick)
        {
            enteringEdges[id] = parent;
        }
        else
        {
            if(enteringEdges.find(parent->getEdgeID()) == enteringEdges.end())
            {
                enteringEdges[id] = parent;
            }
        }
    }

    template <class Data>
    void Node<Data>::addChildEdge(edgePtr<Data> child, const string& id, bool quick)
    {
        if(quick)
        {
            leavingEdges[id] = child;
        }
        else
        {
            if(leavingEdges.find(child->getEdgeID()) == leavingEdges.end())
            {
                leavingEdges[id] = child;
            }
        }
    }

    template <class Data>
    void Node<Data>::addParentNode(nodePtr<Data> parent, const float cost)
    {
        const string edgeSpacer = "-";
        string parentID         = parent->getNodeID();
        string myID             = this->getNodeID();
        const string& edgeID    = parentID + edgeSpacer + myID;
        Edge<Data> parentEdge(edgeID, myID, parentID, cost);
        edgePtr<Data> parentEdgePtr        = edgePtr<Data>(&parentEdge);
        enteringEdges[parent->getNodeID()] = parentEdgePtr;
    }

    template <class Data>
    void Node<Data>::addChildNode(nodePtr<Data> child, const float cost)
    {
        const string edgeSpacer = "-";
        string childID          = child->getNodeID();
        string myID             = this->getNodeID();
        const string& edgeID    = myID + edgeSpacer + childID;
        Edge<Data> childEdge(edgeID, childID, myID, cost);
        edgePtr<Data> childEdgePtr       = edgePtr<Data>(&childEdge);
        leavingEdges[child->getNodeID()] = childEdgePtr;
    }

    template <class Data>
    bool Node<Data>::removeParentNode(const std::string id)
    {
        auto foundNode       = enteringEdges.find(id);
        bool removedFromNode = false;
        if(foundNode != enteringEdges.end())
        {
            enteringEdges.erase(id);
            removedFromNode = true;
        }
        return removedFromNode;
    }

    template <class Data>
    bool Node<Data>::removeChildNode(const std::string id)
    {
        auto foundNode       = leavingEdges.find(id);
        bool removedFromNode = false;
        if(foundNode != leavingEdges.end())
        {
            leavingEdges.erase(id);
            removedFromNode = true;
        }
        return removedFromNode;
    }

    template <class Data>
    void Node<Data>::setID(string newID)
    {
        nodeID = newID;
    }

}  // namespace grstaps

#endif  // GRSTAPS_NODESCPP