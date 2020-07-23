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
 *//*


#ifndef GRSTAPS_ASTARCPP
#define GRSTAPS_ASTARCPP

#include "grstaps/Search/AStarSearch.h"

namespace grstaps {

    template<class Data>
    AStarSearch<Data>::AStarSearch(Graph<Data> &graph, nodePtr<Data> &initPtr) : SearchBase<Data>(graph, initPtr) {
        currentNode = initPtr; // variable the holds the current explored node
        frontier.push(initPtr);
    }

    template<class Data>
    AStarSearch<Data>::AStarSearch(AStarSearch<Data> &p2, NodeExpander<TaskAllocation>* expander) : SearchBase<Data>() {
        this->graph = Graph<Data>();

        for (auto itr = p2.frontier.begin(); itr != p2.frontier.end(); ++itr) {
            nodePtr<Data> nodeToAdd  = nodePtr<Data>(new Node<Data>(itr));
            this->graph.addNode(nodeToAdd);
            frontier.push(nodeToAdd);
        }

        for (auto itr = p2.closedList.begin(); itr != p2.closedList.end(); ++itr) {
            nodePtr<Data> nodeToAdd = nodePtr<Data>(new Node<Data>(itr));
            this->graph.addNode(nodeToAdd);
            closedList.push(nodeToAdd);
        }


        currentNode = this->graph.findNode(p2.currentNode->getNodeID());
        this->initialNodePtr = this->graph.findNode(p2.currentNode->getNodeID());
    }

    template<> AStarSearch<TaskAllocation>::AStarSearch(AStarSearch<TaskAllocation> &p2, short newActionDuration, vector<float>& traitRequirements, vector<float>& nonCumTraits, NodeExpander<TaskAllocation>* expander, vector<vector<int>>* orderingConstraints ) : SearchBase<TaskAllocation>() {
        graph = Graph<TaskAllocation>();
        float goalDistAdd = 0;
        for(int i=0; i < traitRequirements.size(); i++){
            goalDistAdd += traitRequirements[i];
        }

        boost::shared_ptr<vector<vector<float>>> goalTrait = boost::shared_ptr<vector<vector<float>>>( new vector<vector<float>>(*p2.initialNodePtr->getData().getGoalTraitDistribution()));
        boost::shared_ptr<vector<float>> durations = boost::shared_ptr<vector<float>>( new vector<float>(*p2.initialNodePtr->getData().getActionDuration()));
        boost::shared_ptr<vector<vector<int>>> orderings = boost::shared_ptr<vector<vector<int>>>(new vector<vector<int>>(*p2.initialNodePtr->getData().getOrderingConstraints()));

        TaskAllocation copy((p2.currentNode)->getData());
        nodePtr<TaskAllocation> nodeToAdd = nodePtr<TaskAllocation>(new Node<TaskAllocation>(p2.currentNode->getNodeID() + string ( copy.getSpeciesTraitDistribution()->size(), '0'), copy));

        nodeToAdd->getData().setGoalTraitDistribution(goalTrait);
        nodeToAdd->getData().setActionDuration(durations);
        nodeToAdd->getData().setOrderingConstraints(orderings);

        (nodeToAdd->getData()).addAction(traitRequirements, nonCumTraits, goalDistAdd,  newActionDuration, orderingConstraints);
        this->graph.addNode(nodeToAdd);
        frontier.push(nodeToAdd);

        for (auto itr = p2.frontier.ordered_begin(); itr != p2.frontier.ordered_end(); ++itr) {
            TaskAllocation copy((*itr)->getData());
            nodePtr<TaskAllocation> nodeToAdd = nodePtr<TaskAllocation>(new Node<TaskAllocation>((*itr)->getNodeID() + string ( copy.getSpeciesTraitDistribution()->size(), '0'), copy));

            nodeToAdd->getData().setGoalTraitDistribution(goalTrait);
            nodeToAdd->getData().setActionDuration(durations);
            nodeToAdd->getData().setOrderingConstraints(orderings);

            (nodeToAdd->getData()).addAction(traitRequirements, nonCumTraits, goalDistAdd,  newActionDuration, orderingConstraints);
            this->graph.addNode(nodeToAdd);
            frontier.push(nodeToAdd);
        }
        this->initialNodePtr = this->frontier.top();
        this->currentNode = this->frontier.top();
        closedList.push(currentNode);
        frontier.pop();
    }

    template<class Data>
    void AStarSearch<Data>::search(GoalLocator<Data>* goal, NodeExpander<Data>* expander, SearchResultPackager<Data>* results) {
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
    bool AStarSearch<Data>::updateCurrent()  {
        bool searchFailed = false;
        if (frontier.empty()) {
            searchFailed = true;
        }
        else {
            currentNode = frontier.top();
            closedList.push(currentNode);
            frontier.pop();

        }
        return searchFailed;

    }

}//namespace grstaps

#endif*/
