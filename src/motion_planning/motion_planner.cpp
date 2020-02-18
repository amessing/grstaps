/*
 * Copyright (C)2020 Andrew Messing
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
#include "grstaps/motion_planning/motion_planner.hpp"

// external
#include <ompl/base/ProblemDefinition.h>
#include <ompl/base/objectives/PathLengthOptimizationObjective.h>
#include <ompl/geometric/planners/prm/LazyPRM.h>

// local
#include "grstaps/knowledge.hpp"
#include "grstaps/location.hpp"
#include "grstaps/motion_planning/validity_checker.hpp"

namespace grstaps
{
    namespace ob = ompl::base;
    namespace og = ompl::geometric;

    MotionPlanner& MotionPlanner::instance()
    {
        static MotionPlanner rv;
        return rv;
    }

    void MotionPlanner::setMap(const std::vector<b2PolygonShape>& obstacles)
    {
        // Construct the state space in which we are planning: R^2
        m_space = std::make_shared<ob::RealVectorStateSpace>(2);
        // todo: set the bounds based on the map
        m_space->setBounds(0.0, 1.0);

        m_space_information = std::make_shared<ob::SpaceInformation>(m_space);
        m_space_information->setStateValidityChecker(std::make_shared<ValidityChecker>(obstacles, m_space_information));
        m_space_information->setup();

        m_planner = std::make_shared<og::LazyPRM>(m_space_information);

        m_map_set = true;
    }

    void MotionPlanner::setQueryTime(float run_time)
    {
        m_query_time = run_time;
    }

    std::pair<bool, float> MotionPlanner::query(unsigned int from, unsigned int to)
    {
        Knowledge& knowledge = Knowledge::instance();
        return query(knowledge.location(from), knowledge.location(to));
    }

    std::pair<bool, float> MotionPlanner::query(const Location& from, const Location& to)
    {
        // Create the robot's starting state
        ob::ScopedState<> start(m_space);
        start->as<ob::RealVectorStateSpace::StateType>()->values[0] = from.x();
        start->as<ob::RealVectorStateSpace::StateType>()->values[1] = from.y();

        // Create the robot's goal state
        ob::ScopedState<> goal(m_space);
        goal->as<ob::RealVectorStateSpace::StateType>()->values[0] = to.x();
        goal->as<ob::RealVectorStateSpace::StateType>()->values[1] = to.y();

        // Create problem instance
        auto problem = std::make_shared<ob::ProblemDefinition>(m_space_information);
        problem->setStartAndGoalStates(start, goal);
        problem->setOptimizationObjective(std::make_shared<ob::PathLengthOptimizationObjective>(m_space_information));

        m_planner->setProblemDefinition(problem);
        m_planner->setup();
        ob::PlannerStatus solved = m_planner->solve(m_query_time);
        if(solved)
        {
            return std::make_pair(true, problem->getSolutionPath()->length());
        }
        return std::make_pair(false, -1.0);
    }

    MotionPlanner::MotionPlanner()
        : m_map_set(false)
        , m_query_time(1.0)
    {}
}  // namespace grstaps