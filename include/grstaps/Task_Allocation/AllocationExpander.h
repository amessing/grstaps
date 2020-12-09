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

#ifndef GRSTAPS_ALLOCEXPANDER
#define GRSTAPS_ALLOCEXPANDER

// local
#    include <grstaps/Graph/Graph.h>
#    include <grstaps/Graph/Node.h>
#    include <grstaps/Search/Cost.h>
#    include <grstaps/Search/Heuristic.h>
#    include <grstaps/Search/NodeExpander.h>
#    include <grstaps/Task_Allocation/TaskAllocation.h>

namespace grstaps
{
    template <typename Data>
    using nodePtr = typename std::shared_ptr<Node<Data>>;

    /**
     * Functor for expanding the graph by adding children to a node
     *
     *
     *
     */
    class AllocationExpander : public NodeExpander<TaskAllocation>
    {
       public:
        /**
         * Constructor
         *
         * \param the heuristic object
         * \param the cost object
         *
         */
        AllocationExpander(std::shared_ptr<const Heuristic>, std::shared_ptr<const Cost>);

        /**
         * Expands a  graph by adding a nodes children
         *
         * \param the graph
         * \param the node parent
         * \param the node who's children you wish to add
         *
         */
        bool operator()(Graph<TaskAllocation>& graph, nodePtr<TaskAllocation> expandNode) const override;

        /**
         * Gets the id of the new node by editing the parent node
         *
         * \param parent node id
         * \param int representing the task species combo that you are adding too
         *
         */
        static std::string editID(const vector<short>&, const std::string&, int) ;
    };

}  // namespace grstaps
#endif  // GRSTAPS_ALLOCEXPANDER