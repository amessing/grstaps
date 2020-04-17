/*
 * Copyright (C) 2020 Andrew Messing
 *
 * grstaps is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 3 of the License,
 * or any later version.
 *
 * grstaps is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with grstaps; if not, write to the Free Software Foundation,
 * Inc., #59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#ifndef GRSTAPS_PLAN_HPP
#define GRSTAPS_PLAN_HPP

// global
#include <memory>
#include <vector>


namespace grstaps
{
    // Forward Declaration
    class Action;
    class CausalLink;
    class OpenCondition;
    class Ordering;

    /**
     * A partial-order plan
     */
    class Plan
    {
    public:
        /**
         * Constructor
         *
         * \param parent The parent of this plan
         * \param action The new action added
         */
        Plan(Plan* parent, std::shared_ptr<Action> action);

        /**
         * Adds the list of child plans
         */
        void addChildren(const std::vector<std::shared_ptr<Plan>>& children);

        /**
         * \returns Whether this node has been expanded
         */
        inline bool expanded() const
        {
            return m_children.size() > 0;
        }

        /**
         * \returns Whether this plan has open conditions
         */
        inline bool hasOpenConditions() const
        {
            return m_open_conditions.size() > 0;
        }

        /**
         * \returns Whether this is the root node
         */
        inline bool isRoot() const
        {
            return m_parent == nullptr;
        }

        /**
         * \returns Whether this plan is a solution
         */
        bool isSolution() const;

        /**
         * \returns The path cost
         */
        float g() const;

        /**
         * \returns The heuristic value
         */
        float h() const;

        /**
         * \returns The total cost of this plan (f = g + h)
         */
        float f() const;
    private:
        unsigned int m_id;
        Plan* m_parent; //!< The parent of this plan
        std::vector<std::shared_ptr<Plan>> m_children; //!< List of the children of this plan
        std::shared_ptr<Action> m_new_action; //!< The new action added to the parent plan
        std::vector<Ordering> m_orderings; //!< List of the new order
        std::vector<CausalLink> m_causal_links; //!< List of the new causal links
        std::vector<OpenCondition> m_open_conditions; //!< List of the open conditions
        float m_g; //!< The path cost
        float m_h; //!< The heuristic value

        static unsigned int m_next_plan_id; //!< The identifier for the next plan
    };

    /**
     * A comparator for the priority queue in fcpop
     */
    class ComparePlanPtr : public std::less<std::shared_ptr<Plan>>
    {
        bool operator()(const std::shared_ptr<Plan>& lhs, const std::shared_ptr<Plan>& rhs) const;
    };
}

#endif //GRSTAPS_PLAN_HPP
