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

#ifndef GRSTAPS_ASTAR
#define GRSTAPS_ASTAR

// internal
#    include "SearchBase.h"
#    include "grstaps/Graph/Graph.h"
#    include "grstaps/Graph/Node.h"
#    include "grstaps/Search/GoalLocator.h"
#    include "grstaps/Search/NodeExpander.h"
#    include "grstaps/Search/SearchComparators.h"
#    include "grstaps/Search/SearchResultPackager.h"

// external
#    include <boost/heap/binomial_heap.hpp>

namespace grstaps
{
    template <class Data>
    class AStarSearch : public SearchBase<Data>
    {
       public:
        /**
         * Constructor
         *
         * \param the graph that the search is through
         *
         */
        AStarSearch(Graph<Data> &, nodePtr<Data> &);

        /**
         * Constructor
         *
         *  \param the search problem you are going to copy from
         *  \param id of new action
         *  \param traitRequirements of the new action
         *  \param the nonCumTraits requirements
         *
         */
        // AStarSearch(AStarSearch<TaskAllocation> &p2, short newActionDuration, vector<float>& traitRequirements,
        // vector<float>& nonCumTraits, NodeExpander<TaskAllocation>* expander, vector<vector<int>>*
        // orderingConstraints);

        /**
         * Copy Constructor
         *
         * \param the search problem you are going to copy from
         * \param a node expander
         *
         */
        AStarSearch(AStarSearch<Data> &, NodeExpander<TaskAllocation> *);

        /**
         * Search through the graph and find a node
         *
         * \param functor that returns a bool to say if the current node is a goal
         * \param node expander that is a functor that expconst GoalLocator<Data>*, const NodeExpander<Data>*,ands a
         * past node adding its children to the graph \param a functor that formats the solution found \param a bool for
         * checking if allocation is possible before search
         *
         * \returns a search results object that contains all needed return information from the search
         */
        void search(std::shared_ptr<const GoalLocator<TaskAllocation>> goal,
                            std::shared_ptr<const NodeExpander<TaskAllocation>>,
                            SearchResultPackager<Data> *) override;

        /**
         * Update the Current node
         *
         */
        bool updateCurrent();

        //! \returns Whether the frontier is empty
        [[nodiscard]] bool empty() const;

        int nodesExpanded{};
        int nodesSearched{};

       private:
        nodePtr<Data> currentNode;
        boost::heap::binomial_heap<nodePtr<Data>, boost::heap::compare<NodeCompareF<Data>>> frontier;
        boost::heap::binomial_heap<nodePtr<Data>, boost::heap::compare<NodeCompareF<Data>>> closedList;
    };

    template <class Data>
    AStarSearch<Data>::AStarSearch(Graph<Data> &graph, nodePtr<Data> &initPtr)
        : SearchBase<Data>(graph, initPtr)
    {
        currentNode = initPtr;  // variable the holds the current explored node
        frontier.push(initPtr);
        nodesExpanded = 0;
        nodesSearched = 0;
    }

    template <typename Data>
    bool AStarSearch<Data>::empty() const
    {
        return frontier.empty();
    }

    template <class Data>
    bool AStarSearch<Data>::updateCurrent()
    {
        bool searchFailed = false;
        if(frontier.empty())
        {
            searchFailed = true;
        }
        else
        {
            currentNode = frontier.top();
            closedList.push(currentNode);
            frontier.pop();
            nodesExpanded += 1;
        }
        return searchFailed;
    }


    template <class Data>
    AStarSearch<Data>::AStarSearch(AStarSearch<Data> &p2, NodeExpander<TaskAllocation> *expander)
        : SearchBase<Data>()
    {
        this->graph = Graph<Data>();

        for(auto itr = p2.frontier.begin(); itr != p2.frontier.end(); ++itr)
        {
            nodePtr<Data> nodeToAdd = nodePtr<Data>(new Node<Data>(itr));
            this->graph.addNode(nodeToAdd);
            frontier.push(nodeToAdd);
        }

        for(auto itr = p2.closedList.begin(); itr != p2.closedList.end(); ++itr)
        {
            nodePtr<Data> nodeToAdd = nodePtr<Data>(new Node<Data>(itr));
            this->graph.addNode(nodeToAdd);
            closedList.push(nodeToAdd);
        }

        currentNode          = this->graph.findNode(p2.currentNode->getNodeID());
        this->initialNodePtr = this->graph.findNode(p2.currentNode->getNodeID());
    }

    template <class Data>
    void AStarSearch<Data>::search(std::shared_ptr<const GoalLocator<TaskAllocation>> goal,
                                   std::shared_ptr<const NodeExpander<TaskAllocation>> expander,
                                   SearchResultPackager<Data> *results)
    {
        bool searchFailed = updateCurrent();
        while(!searchFailed)
        {
            if((*goal)(this->graph, currentNode))
            {
                results->addResults(this->graph, currentNode, searchFailed);
                return;
            }

            (*expander)(this->graph, this->currentNode);
            //float currentCost = this->currentNode->getPathCost();
            nodesSearched += this->currentNode->leavingEdges.size();
            for(auto itr = this->currentNode->leavingEdges.begin(); itr != this->currentNode->leavingEdges.end(); ++itr)
            {
                auto node = this->graph.findNode(itr->second->getTailNode());
                frontier.push(node);
            }
            searchFailed = updateCurrent();
        }
        results->addResults(this->graph, currentNode, searchFailed);
    }

}  // namespace grstaps
#endif  // GRSTAPS_ASTAR
