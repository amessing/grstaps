/*
 * Copyright (C) 2021 Andrew Messing
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
#include "grstaps/solver_base.hpp"

#include <nlohmann/json.hpp>
#include <boost/make_shared.hpp>
#include <box2d/b2_polygon_shape.h>

#include "grstaps/problem.hpp"
#include "grstaps/task_planning/plan.hpp"
#include "grstaps/motion_planning/motion_planner.hpp"

namespace grstaps
{
    void SolverBase::planSubcomponents(const Plan *base, std::vector<const Plan *> &plan_subcomponents)
    {
        if(base == nullptr)
        {
            plan_subcomponents.clear();
        }
        else
        {
            planSubcomponents(base->parentPlan, plan_subcomponents);
            plan_subcomponents.push_back(base);
        }
    }
    boost::shared_ptr<std::vector<boost::shared_ptr<MotionPlanner>>> SolverBase::setupMotionPlanners(
        const Problem &problem)
    {
        const std::vector<ClipperLib2::Paths>& maps =  problem.map();
        const nlohmann::json& config = problem.config();

        auto motion_planners = boost::make_shared<std::vector<boost::shared_ptr<MotionPlanner>>>();
        motion_planners->reserve(maps.size());

        const float boundary_min      = config["mp_boundary_min"];
        const float boundary_max      = config["mp_boundary_max"];
        const float query_time        = config["mp_query_time"];
        const float connection_range  = config["mp_connection_range"];

        for(int i = 0; i < maps.size(); ++i)
        {
            auto motion_planner = boost::make_shared<MotionPlanner>();
            motion_planner->setMap(maps[i], boundary_min, boundary_max);
            motion_planner->setLocations(problem.locations());
            motion_planner->setQueryTime(query_time);
            motion_planner->setConnectionRange(connection_range);
            motion_planners->push_back(motion_planner);
        }
        return motion_planners;
    }
    void SolverBase::setupTaskAllocationParameters(
        const Plan *plan,
        const Problem &problem,
        boost::shared_ptr<std::vector<std::vector<int>>> ordering_constraints,
        boost::shared_ptr<std::vector<float>> durations,
        boost::shared_ptr<std::vector<std::vector<float>>> noncum_trait_cutoff,
        boost::shared_ptr<std::vector<std::vector<float>>> goal_distribution,
        boost::shared_ptr<std::vector<std::pair<unsigned int, unsigned int>>> action_locations)
    {
        // Fill in vectors for TA and Scheduling
        std::vector<const Plan*> plan_subcomponents;
        planSubcomponents(plan, plan_subcomponents);
        std::set<std::pair<uint16_t, uint16_t>> order_constraints;
        for(unsigned int j = 0; j < plan_subcomponents.size(); ++j)
        {
            const Plan* subcomponent = plan_subcomponents[j];
            // Ignore the fictitious action
            // TODO: ignore TILs?
            if(subcomponent->action->name != "<goal>" && subcomponent->action->name != "#initial")
            {
                for(unsigned int k = 0; k < subcomponent->orderings.size(); ++k)
                {
                    // uint16_t
                    TTimePoint fp = firstPoint(subcomponent->orderings[k]);
                    TTimePoint sp = secondPoint(subcomponent->orderings[k]);
                    // Time points are based on start and end snap actions
                    // Also remove the initial action
                    order_constraints.insert({fp / 2 - 1, sp / 2 - 1});
                }

                durations->push_back(subcomponent->action->duration[0].exp.value);

                noncum_trait_cutoff->push_back(
                    problem.actionNonCumRequirements[problem.actionToRequirements.at(subcomponent->action->name)]);
                goal_distribution->push_back(
                    problem.actionRequirements[problem.actionToRequirements.at(subcomponent->action->name)]);
                action_locations->push_back(problem.actionLocation(subcomponent->action->name));
            }
        }

        for(auto oc: order_constraints)
        {
            ordering_constraints->push_back({oc.first, oc.second});
        }
    }
}