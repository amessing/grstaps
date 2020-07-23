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

#ifndef GRSTAPS_ALLOCEXPANDERCPP
#define GRSTAPS_ALLOCEXPANDERCPP

#include "grstaps/Task_Allocation/AllocationExpander.h"


namespace grstaps
{


    template <typename Data> using nodePtr = typename boost::shared_ptr<Node<Data>>;

    AllocationExpander::AllocationExpander(boost::shared_ptr<Heuristic> heur, boost::shared_ptr<Cost> cos): NodeExpander(heur, cos){}

    //check to prevent duplicate
    bool AllocationExpander::operator()(Graph<TaskAllocation>& graph, nodePtr<TaskAllocation>& expandNode)
    {
        TaskAllocation data = expandNode->getData();
        vector<short> allocation = data.getAllocation();
        std::string nodeID = expandNode->getNodeID();
        float currentCost = expandNode->getPathCost();
        int numSpecies = data.getNumSpecies()->size();
        float parentsGoalDistance = data.getGoalDistance();

        int numTask = allocation.size() / numSpecies;
        vector<int> numSpec = *data.getNumSpecies();
        for(int i = 0; i < numTask; ++i)
        {
            for(int j = 0; j < numSpecies; ++j)
            {
                int index = i * numSpecies + j;
                std::string newNodeID = editID(allocation, nodeID, index);

                if(!graph.nodeExist(newNodeID) && (int(newNodeID[(i * numSpecies) + j] - '0') <= (numSpec)[j]))
                {
                    TaskAllocation newNodeData(data);
                    newNodeData.addAgent(j, i);
                    if(newNodeData.getGoalDistance() < parentsGoalDistance)
                    {
                        float heur = (*this->heuristicFunc)(graph, data, newNodeData);
                        float cost = (*this->costFunc)(graph, data, newNodeData);
                        auto newNode = nodePtr<TaskAllocation>(new Node<TaskAllocation>(newNodeID, newNodeData));
                        newNode->setHeuristic(heur);
                        newNode->setPathCost(heur);
                        graph.addNode(newNode, newNodeID);
                        graph.addEdge(newNodeID, nodeID, heur - currentCost, expandNode, newNode);
                    }
                }
            }

        }

        return true;

    }

    std::string AllocationExpander::editID(vector<short>& allocation, std::string parentNodeID, int indexExp)
    {
        int idSize = parentNodeID.length() / (allocation.size());
        string newNodeID = parentNodeID;
        int index = idSize * indexExp + idSize - 1;
        int add = 1;
        for(int i = 0; i < idSize; i++)
        {
            int newDigit = int(newNodeID[index]) - 48 + add;
            if(newDigit == 10)
            {
                newDigit = 0;
            }
            else
            {
                newNodeID[index] = char(newDigit + 48);
                index--;
                add = 0;
            }
        }
        return newNodeID;
    }

}//grstaps

#endif
