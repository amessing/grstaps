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
// external

#include <gtest/gtest.h>

// local
#include <grstaps/motion_planning/motion_planner.hpp>

namespace grstaps
{
    namespace test
    {
        TEST(MotionPlanning, test1)
        {
            std::vector<b2PolygonShape> obstacles;

            // Obstacle 1
            {
                b2PolygonShape obstacle;

                b2Vec2 vertices[6];

                obstacle.Set(vertices, 6);
                obstacles.push_back(obstacle);
            }

            // Obstacle 2// external
            {
                b2PolygonShape obstacle;

                b2Vec2 vertices[6];

                obstacle.Set(vertices, 6);
                obstacles.push_back(obstacle);
            }

            // Obstacle 3
            {
                b2PolygonShape obstacle;

                b2Vec2 vertices[6];

                obstacle.Set(vertices, 6);
                obstacles.push_back(obstacle);
            }

            // Obstacle 4
            {
                b2PolygonShape obstacle;

                b2Vec2 vertices[6];

                obstacle.Set(vertices, 6);
                obstacles.push_back(obstacle);
            }

            //MotionPlanner& mp = MotionPlanner::instance();
            //mp.setMap(obstacles, 0, 10);
        }
    }  // namespace test
}  // namespace grstaps