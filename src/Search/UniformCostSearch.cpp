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
#ifndef GRSTAPS_UNIFORMCOSTCPP
#define GRSTAPS_UNIFORMCOSTCPP

//internal
#include "grstaps/Search/UniformCostSearch.h"
#include "grstaps/Graph/Node.h"
#include "grstaps/Graph/Graph.h"
#include "grstaps/Search/GoalLocator.h"
#include "grstaps/Search/NodeExpander.h"
#include "grstaps/Search/SearchResultPackager.h"
#include "grstaps/Search/SearchComparators.h"

//external
#include <boost/heap/binomial_heap.hpp>

namespace grstaps {

    template<class Data>
    UniformCostSearch<Data>::UniformCostSearch(Graph<Data> &graph, nodePtr<Data> &initPtr) : SearchBase<Data>(graph, initPtr) {
        currentNode = initPtr; // variable the holds the current explored node
        frontier.push(initPtr);
    }

    template<class Data>
    UniformCostSearch<Data>::UniformCostSearch(UniformCostSearch<Data> &p2, NodeExpander<Data> &expandGraph) : SearchBase<Data>() {
        for (auto itr = p2.frontier.begin(); itr != p2.frontier.leavingEdges.end(); ++itr) {
            nodePtr<Data> nodeToAdd = nodePtr<Data>(new Node<Data>(itr));
            this->graph.addNode(nodeToAdd);
            frontier.push(nodeToAdd);
        }
        for (auto itr = closedList.begin(); itr != closedList.end(); ++itr) {
            nodePtr<Data> nodeToExpand = nodePtr<Data>(new Node<Data>(itr));
            expandGraph(this->graph, nodeToExpand);
        }

        currentNode = this->graph.findNode(p2->currentNode->getNodeID());
        this->initialNodePtr = this->graph.findNode(p2->initialNodePtr->getNodeID());
    }
    template<class Data>
    void UniformCostSearch<Data>::search(GoalLocator<Data>* goal, NodeExpander<Data>* expander, SearchResultPackager<Data>* results) {
        GoalLocator<Data>& isGoal = *goal;  // Functor that tells us if a node is a goal
        NodeExpander<Data>& expandGraph = *expander;  // Functor that takes a node and a graph and expands that node to add children
        bool searchFailed = updateCurrent();
        while (!searchFailed && !isGoal(this->graph, currentNode)) {
            expandGraph(this->graph, this->currentNode);
            float currentCost = this->currentNode->getPathCost();
            for (auto itr = this->currentNode->leavingEdges.begin(); itr != this->currentNode->leavingEdges.end(); ++itr) {
                auto node = this->graph.findNode(itr->second->getTailNode());
                frontier.push(node);
            }
            searchFailed = updateCurrent();
        }

        results->addResults(this->graph, currentNode, searchFailed);

    }
    template<class Data>
    bool UniformCostSearch<Data>::updateCurrent()  {
        bool searchFailed = false;
        if (frontier.empty()) {
            searchFailed = true;
        }
        else {
            currentNode = frontier.top();
            frontier.pop();
            closedList.push(currentNode);


        }
        return searchFailed;

    }

}//namespace grstaps

#endif