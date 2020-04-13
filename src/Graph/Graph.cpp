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
 * Inc., #59 Temple Place, Suite 330, Boston, MA 021#ifndef GRAPH_CPP11-1307 USA
 */

#ifndef GRAPH_CPP
#define GRAPH_CPP

#include <exception>
#include <fstream>
#include <iomanip>
#include "grstaps/Graph/Graph.h"


using namespace boost;

namespace grstaps {

    template<class Data>
    nodePtr<Data> Graph<Data>::findNode(const string &id) const {
        auto foundNode = nodeList.find(id);
        if (foundNode != nodeList.end()) {
            return foundNode->second;
        }
        return nodePtr<Data>(NULL);
    }

    template<class Data>
    void Graph<Data>::print() const {

        std::cout << "\n\nGraph\n-----\n";

        for (auto it = nodeList.begin(); it != nodeList.end(); ++it) {
            std::cout << "Node : " << it->second->getNodeID() << std::endl;
            std::cout << "   Parents:" << std::endl;
            it->second->printParents();
            std::cout << "   Children:" << std::endl;
            it->second->printChildren();
            std::cout << std::endl;
        }

        std::cout << std::endl;
    }

    template<class Data>
    int Graph<Data>::printNodeList() const {
        int count = 0;
        std::cout << "Nodes : \n";
        for (auto it = nodeList.begin(); it != nodeList.end(); ++it, ++count) {
            std::cout << std::setw(2) << count << " : " << it->second->getNodeID() << std::endl;
        }
        return --count;
    }

    template<class Data>
    void Graph<Data>::clearSearchState() {
        // iterates over all nodes and clears their search states (parent node, cost, etc.)
        // so the graph can be re-used for a T search

        for (auto it = nodeList.begin(); it != nodeList.end(); ++it) {
            it->second->clearSearchState();
        }
    }

    template<class Data>
    void Graph<Data>::addNode(nodePtr<Data>& nodeToAdd, bool quick) {
        string id = nodeToAdd->getNodeID();
        if(!quick) {
            // before adding the node, try to find it
            nodePtr<Data> nodeExists = findNode(id);
            // if the node already exists, throw an error message
            // if the node does not exist, add it to the list of nodes
            if (nodeList.size() > 0 && nodeExists != NULL) {
                string errorString =
                        "File format error : Duplicate node '" + id + " Node Not Added";
                std::cout << errorString << std::endl;
            } else {
                nodeList[id] = nodeToAdd;
            }
        }
        else{
            nodeList[id] = nodeToAdd;
        }
    }

    template<class Data>
    void Graph<Data>::addNode(nodePtr<Data>& nodeToAdd, string &id, bool quick) {
        if(!quick) {
            // before adding the node, try to find it
            nodePtr<Data> nodeExists = findNode(id);
            // if the node already exists, throw an error message
            // if the node does not exist, add it to the list of nodes
            if (nodeList.size() > 0 && nodeExists != NULL) {
                string errorString =
                        "File format error : Duplicate node '" + id + " Node Not Added";
                std::cout << errorString << std::endl;
            } else {
                nodeList[id] = nodeToAdd;
            }
        }
        else{
            nodeList[id] = nodeToAdd;
        }
    }

    template<class Data>
    void Graph<Data>::addNode(const string& id, Data data, bool quick) {
        if (quick) {
            nodeList[id] = nodePtr<Data>(new Node<Data>(id, data));
        }
        else {
            // before adding the node, try to find it
            nodePtr<Data> nodeExists = findNode(id);
            // if the node already exists, throw an error message
            // if the node does not exist, add it to the list of nodes
            if (nodeList.size() > 0 && nodeExists != NULL) {
                string errorString = "File format error : Duplicate node '" + id + " Node Not Added";
                std::cout << errorString << std::endl;
            } else {
                nodeList[id] = nodePtr<Data>(new Node<Data>(id, data));
            }
        }
    }

    template<class Data>
    void Graph<Data>::addNode(const string &id, Data data, float cost, float heur, int quick) {
        if(quick){
            auto newNode = nodePtr<Data>(new Node<Data>(id, data));
            nodeList[id] = newNode;
            newNode->setHeuristic(heur);
            newNode->setPathCost(cost);
        }
        else {
            // before adding the node, try to find it
            nodePtr<Data> nodeExists = findNode(id);
            // if the node already exists, throw an error message
            // if the node does not exist, add it to the list of nodes
            if (nodeList.size() > 0 && nodeExists != NULL) {
                string errorString =
                        "File format error : Duplicate node '" + id + " Node Not Added";
                std::cout << errorString << std::endl;
            } else {
                auto newNode = nodePtr<Data>(new Node<Data>(id, data));
                nodeList[id] = newNode;
                newNode->setHeuristic(heur);
                newNode->setPathCost(cost);

            }
        }
    }

    template<class Data>
    void Graph<Data>::addEdge(const string &tail, const string &head, float cost, bool quick) {
        // find the pointers to nodes at head and tail, based on the strings
        // throw errors if either is not found
        const string edgeSpacer  = "-"; //!< when id'ing the edges the edge id is the *headNodeID*+edgeSpacer+*tailNodeID"
        nodePtr<Data> nodeTail = findNode(tail);
        nodePtr<Data> nodeHead = findNode(head);
        string edgeId = head + edgeSpacer + tail;
        if(!quick) {

            if (nodeTail == NULL) {
                string errorString = "File error : Node " + tail + " does not exist.";
                throw std::runtime_error(errorString.c_str());
            }

            if (nodeHead == NULL) {
                string errorString = "File error : Node " + head + " does not exist.";
                throw std::runtime_error(errorString.c_str());
            }
            if (edgeExist(edgeId)) {
                string errorString = "File error : Edge already exist.";
                throw std::runtime_error(errorString.c_str());
            }
        }
        edgePtr<Data> newEdge = edgePtr<Data>(new Edge<Data>(tail, head, cost));
        edgeList[edgeId] = newEdge;
        nodeHead->addChildEdge(newEdge);
        nodeTail->addParentEdge(newEdge);
    }

    template<class Data>
    void Graph<Data>::addEdge(const string &tail, const string &head, float cost, nodePtr<Data>& nodeHead, nodePtr<Data>& nodeTail,  bool quick) {
        // find the pointers to nodes at head and tail, based on the strings
        // throw errors if either is not found
        const string edgeSpacer  = "-"; //!< when id'ing the edges the edge id is the *headNodeID*+edgeSpacer+*tailNodeID"
        string edgeId = head + edgeSpacer + tail;
        if(!quick) {

            if (nodeTail == NULL) {
                string errorString = "File error : Node " + tail + " does not exist.";
                throw std::runtime_error(errorString.c_str());
            }

            if (nodeHead == NULL) {
                string errorString = "File error : Node " + head + " does not exist.";
                throw std::runtime_error(errorString.c_str());
            }
            if (edgeExist(edgeId)) {
                string errorString = "File error : Edge already exist.";
                throw std::runtime_error(errorString.c_str());
            }
        }
        edgePtr<Data> newEdge = edgePtr<Data>(new Edge<Data>(tail, head, cost));
        edgeList[edgeId] = newEdge;
        nodeHead->addChildEdge(newEdge, edgeId);
        nodeTail->addParentEdge(newEdge, edgeId);
    }

    template<class Data>
    void Graph<Data>::addEdge(edgePtr<Data> edgeToAdd, bool quick) {
        // find the pointers to nodes at head and tail, based on the strings
        // throw errors if either is not found
        const string head = edgeToAdd->getHeadNode();
        const string tail = edgeToAdd->getTailNode();

        nodePtr<Data> nodeTail = findNode(tail);
        nodePtr<Data> nodeHead = findNode(head);
        if(!quick) {
            if (nodeTail == NULL) {
                string errorString = "File error : Node " + tail + " does not exist.";
                throw std::runtime_error(errorString.c_str());
            }

            if (nodeHead == NULL) {
                string errorString = "File error : Node " + head + " does not exist.";
                throw std::runtime_error(errorString.c_str());
            }

            edgePtr<Data> edgeExist = edgeList.find(edgeToAdd->getEdgeID());
            if (edgeExist != NULL) {
                string errorString = "File error : Edge already exist.";
                throw std::runtime_error(errorString.c_str());
            }
        }
        edgeList[edgeToAdd->getEdgeID()] = edgeToAdd;
        nodeHead->addChildEdge(edgeToAdd);
        nodeTail->addParentEdge(edgeToAdd);
    }

    template<class Data>
    bool Graph<Data>::removeNode(const string id) {
        auto foundNode = nodeList.find(id);
        bool removedFromNode = false;
        if (foundNode != nodeList.end()) {
            //remove all edges that used to be attached to that node


            nodeList.erase(id);
            removedFromNode = true;
        }
        return removedFromNode;
    }

    template<class Data>
    bool Graph<Data>::removeEdge(const string id) {
        auto foundEdge = edgeList.find(id);
        bool removedFromNode = false;
        if (foundEdge != edgeList.end()) {
            //remove all edges that used to be attached to that node

            nodeList.find(foundEdge->second->getHeadNode())->second->removeChildNode(id);
            nodeList.find(foundEdge->second->getTailNode())->second->removeParentNode(id);
            edgeList.erase(id);
            removedFromNode = true;
        }
        return removedFromNode;
    }

    template<class Data>
    bool Graph<Data>::nodeExist(string id){
        return nodeList.end() != nodeList.find(id);
    }

    template<class Data>
    bool Graph<Data>::edgeExist(string id){
        auto edgeExist = edgeList.find(id);
        if (edgeExist != edgeList.end()) {
            return true;
        }
        return false;
    }


} // namespace grstaps

#endif