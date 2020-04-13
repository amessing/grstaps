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
        for (auto itr = p2.frontier.begin(); itr != p2.frontier.end(); ++itr) {
            nodePtr<Data> nodeToAdd = nodePtr<Data>(new Node<Data>(itr));
            this->graph.addNode(nodeToAdd);
            frontier.push(nodeToAdd);
        }
        for (auto itr = closedList.begin(); itr != closedList.end(); ++itr) {
            nodePtr<Data> nodeToExpand = nodePtr<Data>(new Node<Data>(itr));
            (*expander)(this->graph, nodeToExpand);
        }

        currentNode = this->graph.findNode(p2->currentNode->getNodeID());
        this->initialNodePtr = this->graph.findNode(p2->initialNodePtr->getNodeID());
    }

    template<> AStarSearch<TaskAllocation>::AStarSearch(AStarSearch<TaskAllocation> &p2, std::string& newActionID, vector<float>& traitRequirements, vector<float>& nonCumTraits, NodeExpander<TaskAllocation>* expander ) : SearchBase<TaskAllocation>() {
        graph = Graph<TaskAllocation>();
        boost::shared_ptr<TaskAllocation> nodeData = boost::shared_ptr<TaskAllocation>(new TaskAllocation(TaskAllocation(p2.initialNodePtr->getData())));
        vector<vector<float>> actionRequirements = (*nodeData->getGoalTraitDistribution());
        actionRequirements.push_back(traitRequirements);
        vector<vector<float>> nonCumActionRequirements = (*nodeData->getActionNoncumulativeTraitValue());
        int numSpec = (nodeData->getSpeciesTraitDistribution()->size());
        nonCumActionRequirements.push_back(nonCumTraits);
        auto newActionIDs = nodeData->getActionIDs();
        this->graph.printNodeList();
        this->graph.printNodeList();
        if(newActionIDs != NULL) {
            newActionIDs->push_back(newActionID);
        }

        float goalDistAdd = 0;
        for(int i=0; i < traitRequirements.size(); i++){
            goalDistAdd += traitRequirements[i];
        }
        nodeData->addAction(&actionRequirements, &nonCumActionRequirements, goalDistAdd);

        for (auto itr = p2.frontier.begin(); itr != p2.frontier.end(); ++itr) {
            nodePtr<TaskAllocation> nodeToAdd = nodePtr<TaskAllocation>(new Node<TaskAllocation>(**itr));
            nodeToAdd->setID((nodeToAdd->getNodeID() + string ( nodeToAdd->getData().getSpeciesTraitDistribution()->size(), '0')));
            (nodeToAdd->getData()).addAction((&actionRequirements), (&nonCumActionRequirements), goalDistAdd,  newActionIDs);
            this->graph.addNode(nodeToAdd);
            frontier.push(nodeToAdd);
        }
        for (auto itr = p2.closedList.begin(); itr != p2.closedList.end(); ++itr) {
            for (int i=0; i < numSpec; i++){
                auto newTA = nodeData;
                vector<short> allocation = (**itr).getData().getAllocation();
                vector<short> emptyVect(numSpec, 0);
                allocation.insert(allocation.end(), emptyVect.begin(), emptyVect.end());
                newTA->setAllocation(allocation);

                newTA->addAgent(i, actionRequirements.size()-1);
                std::string newID = newTA->getID();
                nodePtr<TaskAllocation> newNode = nodePtr<TaskAllocation>(new Node<TaskAllocation> (newID, *newTA));
                this->graph.addNode(newNode);
                frontier.push(newNode);
            }
        }
        this->initialNodePtr = this->frontier.top();
        updateCurrent();

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
            cout << currentNode->getData().getID() << "=" << isGoal(this->graph, currentNode) << endl;
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

#endif