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
 * Inc., #59 Temple Plac
 */

// global
#include <memory>

// external
#include <fmt/format.h>
#include <gtest/gtest.h>

#include <box2d/b2_polygon_shape.h>
#include <ompl/base/Planner.h>
#include <ompl/base/ProblemDefinition.h>
#include <ompl/base/SpaceInformation.h>
#include <ompl/base/StateSpace.h>
#include <ompl/base/objectives/PathLengthOptimizationObjective.h>
#include <ompl/base/spaces/RealVectorStateSpace.h>
#include <ompl/geometric/planners/prm/LazyPRM.h>

// local
#include <grstaps/motion_planning/motion_planner.hpp>
#include <grstaps/motion_planning/validity_checker.hpp>

namespace ob = ompl::base;
namespace og = ompl::geometric;

namespace grstaps
{
    namespace test
    {
        /**
         * @see ompl.kavrakilab.org./OptimalPlanning_8cpp_source.html
         */
        TEST(MotionPlanning, isolated)
        {
            std::vector<b2PolygonShape> obstacles;
            Location from("source", 0.5, 0.5);
            Location to("target", 1.5, 1.5);
            double runtime = 1.0;

            // Construct the state space R^2
            auto space = std::make_shared<ob::RealVectorStateSpace>(2);

            // Set the bounds of the space [0, 2] x [0, 2]
            space->setBounds(0.0, 2.0);

            // Construct the space information
            auto space_information = std::make_shared<ob::SpaceInformation>(space);

            // Set the object used to check which states in the space are valid
            space_information->setStateValidityChecker(std::make_shared<ValidityChecker>(obstacles, space_information));
            space_information->setup();

            // Set the robot's starting state
            ob::ScopedState<> start(space);
            start->as<ob::RealVectorStateSpace::StateType>()->values[0] = from.x();
            start->as<ob::RealVectorStateSpace::StateType>()->values[1] = from.y();

            // Set the robot's goal state
            ob::ScopedState<> goal(space);
            goal->as<ob::RealVectorStateSpace::StateType>()->values[0] = to.x();
            goal->as<ob::RealVectorStateSpace::StateType>()->values[1] = to.y();

            auto problem = std::make_shared<ob::ProblemDefinition>(space_information);
            problem->setStartAndGoalStates(start, goal);
            problem->setOptimizationObjective(std::make_shared<ob::PathLengthOptimizationObjective>(space_information));

            ob::PlannerPtr planner = std::make_shared<og::LazyPRM>(space_information);
            planner->setup();

            planner->setProblemDefinition(problem);

            ob::PlannerStatus solved = planner->solve(runtime);
            if(solved)
            {
                std::cout << fmt::format(
                    "{} found a solution of length {}", planner->getName(), problem->getSolutionPath()->length());
            }
            else
            {
                FAIL();
            }
        }

        TEST(MotionPlanning, p1)
        {
            std::vector<b2PolygonShape> obstacles;
            Location from("source", 0.5, 0.5);
            Location to("target", 1.5, 1.5);
            double runtime = 0.001;

            std::vector<Location> locations = {from, to};

            auto& mp = MotionPlanner::instance();
            mp.setMap(obstacles, 0.0, 2.0);
            mp.setLocations(locations);
            mp.setQueryTime(runtime);
            mp.setConnectionRange(0.1);
            std::pair<bool, float> result = mp.query(0, 1);
            if(result.first)
            {
                std::cout << result.second << std::endl;
            }
            else
            {
                FAIL();
            }
        }

        TEST(MotionPlanning, p1_waypoints)
        {
            std::vector<b2PolygonShape> obstacles;
            Location from("source", 0.5, 0.5);
            Location to("target", 1.5, 1.5);
            double runtime = 0.0001;

            std::vector<Location> locations = {from, to};

            auto& mp = MotionPlanner::instance();
            mp.setMap(obstacles, 0.0, 2.0);
            mp.setLocations(locations);
            mp.setQueryTime(runtime);
            mp.setConnectionRange(0.1);
            std::pair<bool, float> result = mp.query(0, 1);
            if(result.first)
            {
                std::cout << result.second << std::endl;
                std::vector<std::pair<float, float>> waypoints = mp.getWaypoints(0, 1);
                std::cout << waypoints.size() << std::endl;
                for(const std::pair<float, float>& waypoint: waypoints)
                {
                    std::cout << '\t' << waypoint.first << ", " << waypoint.second << std::endl;
                }
            }
            else
            {
                FAIL();
            }
        }

        TEST(MotionPlanning, p1_obstacles)
        {
            std::vector<b2PolygonShape> obstacles;

            // Block the direct path
            b2PolygonShape obstacle;
            obstacle.SetAsBox(0.1, 0.1, b2Vec2(1.0, 1.0), 0);
            obstacles.push_back(obstacle);

            Location from("source", 0.5, 0.5);
            Location to("target", 1.5, 1.5);
            double runtime = 0.0001;

            std::vector<Location> locations = {from, to};

            auto& mp = MotionPlanner::instance();
            mp.setMap(obstacles, 0.0, 2.0);
            mp.setLocations(locations);
            mp.setQueryTime(runtime);
            mp.setConnectionRange(0.1);
            std::pair<bool, float> result = mp.query(0, 1);
            if(result.first)
            {
                std::cout << result.second << std::endl;
                std::vector<std::pair<float, float>> waypoints = mp.getWaypoints(0, 1);
                std::cout << waypoints.size() << std::endl;
                for(const std::pair<float, float>& waypoint: waypoints)
                {
                    std::cout << '\t' << waypoint.first << ", " << waypoint.second << std::endl;
                }
            }
            else
            {
                FAIL();
            }
        }
    }  // namespace test
}  // namespace grstaps
