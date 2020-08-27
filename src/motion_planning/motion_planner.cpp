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
#include <ompl/base/spaces/RealVectorStateSpace.h>
#include <ompl/geometric/planners/prm/LazyPRMstar.h>

// local
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

    void MotionPlanner::setMap(const std::vector<b2PolygonShape>& obstacles, float boundary_min, float boundary_max)
    {
        // Construct the state space in which we are planning: R^2
        m_space = std::make_shared<ob::RealVectorStateSpace>(2);

        // Set the boundary [min, max]
        std::dynamic_pointer_cast<ob::RealVectorStateSpace>(m_space)->setBounds(boundary_min, boundary_max);

        // Create the space information
        m_space_information = std::make_shared<ob::SpaceInformation>(m_space);
        m_space_information->setStateValidityChecker(std::make_shared<ValidityChecker>(obstacles, m_space_information));
        m_space_information->setup();

        // Create the LazyPRMStar planner
        m_planner = std::make_shared<og::LazyPRMstar>(m_space_information);
        m_planner->setup();

        m_map_set = true;
    }

    void MotionPlanner::setQueryTime(float run_time)
    {
        m_query_time = run_time;
    }

    void MotionPlanner::setConnectionRange(float range)
    {
        if(!m_map_set)
        {
            // Custom exception
            throw "Cannot set connection range before setting the map";
        }
        std::dynamic_pointer_cast<og::LazyPRMstar>(m_planner)->setRange(range);
    }

    void MotionPlanner::setLocations(const std::vector<Location>& locations)
    {
        m_locations = locations;
    }

    std::pair<bool, float> MotionPlanner::query(unsigned int from, unsigned int to)
    {
        assert(from < m_locations.size() && to < m_locations.size());
        return query(m_locations[from], m_locations[to]);
    }

    std::pair<bool, float> MotionPlanner::query(const Location& from, const Location& to)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
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

        // Clear the previous problem definition
        std::dynamic_pointer_cast<og::LazyPRMstar>(m_planner)->clearQuery();
        m_planner->setProblemDefinition(problem);

        ob::PlannerStatus solved = m_planner->solve(m_query_time);
        if(solved)
        {
            return std::make_pair(true, problem->getSolutionPath()->length());
        }
        return std::make_pair(false, -1.0);
    }

    std::tuple<bool, float, std::vector<std::pair<float, float>>> MotionPlanner::getWaypoints(unsigned int from, unsigned int to)
    {
        if(from == to)
        {
            return std::make_tuple(false, -1, std::vector<std::pair<float, float>>());
        }

        auto problem = std::make_shared<ob::ProblemDefinition>(m_space_information);
        waypointQuery(from, to, problem);

        ob::PathPtr path               = problem->getSolutionPath();
        auto path_geometric            = path->as<og::PathGeometric>();
        std::vector<ob::State*> states = path_geometric->getStates();

        std::vector<std::pair<float, float>> waypoints;
        for(ob::State* state: states)
        {
            auto& real_vector_state = *state->as<ob::RealVectorStateSpace::StateType>();
            const float x           = real_vector_state[0];
            const float y           = real_vector_state[1];
            if(waypoints.size() > 1 && waypoints.back().first == x && waypoints.back().second == y)
            {
                continue;
            }
            waypoints.push_back(std::make_pair(x, y));
        }
        return std::make_tuple(true, problem->getSolutionPath()->length(), waypoints);
    }

    MotionPlanner::MotionPlanner()
        : m_map_set(false)
        , m_query_time(0.25)
    {}

    void MotionPlanner::waypointQuery(unsigned int from, unsigned int to, ompl::base::ProblemDefinitionPtr problem_def)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        // Create the robot's starting state
        ob::ScopedState<> start(m_space);
        start->as<ob::RealVectorStateSpace::StateType>()->values[0] = m_locations[from].x();
        start->as<ob::RealVectorStateSpace::StateType>()->values[1] = m_locations[from].y();

        // Create the robot's goal state
        ob::ScopedState<> goal(m_space);
        goal->as<ob::RealVectorStateSpace::StateType>()->values[0] = m_locations[to].x();
        goal->as<ob::RealVectorStateSpace::StateType>()->values[1] = m_locations[to].y();

        // Create problem instance
        problem_def->setStartAndGoalStates(start, goal);
        problem_def->setOptimizationObjective(
            std::make_shared<ob::PathLengthOptimizationObjective>(m_space_information));

        // Clear the previous problem definition
        std::dynamic_pointer_cast<og::LazyPRMstar>(m_planner)->clearQuery();
        m_planner->setProblemDefinition(problem_def);

        ob::PlannerStatus solved = m_planner->solve(m_query_time);
        if(solved)
        {
            return;
        }
        // Custom exception needed
        throw "Path could not be made";
    }
}  // namespace grstaps
