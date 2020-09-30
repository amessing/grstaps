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

// external
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <chrono>

#include <nlohmann/json.hpp>

// local
#include <grstaps/problem.hpp>
#include <grstaps/solver.hpp>

namespace grstaps
{
    namespace test
    {
        /**
         * 3 robots start at the source with 3 boxes to bring to the target
         *
         * \note Basic test
         */
        TEST(Problem, p1)
        {
            Problem problem;
            problem.init("tests/data/p1/domain.pddl", "tests/data/p1/problem.pddl", "tests/data/p1/parameters.json");

            problem.configureActions([](const std::vector<SASAction>& actions, Problem* p)
            {
              // All Actions have the same requirements
              for(unsigned int i = 0; i < actions.size(); ++i)
              {
                  p->actionToRequirements[actions[i].name] = i;
                  p->actionRequirements.push_back({.24, 0, 0});
                  p->actionNonCumRequirements.push_back({0, 0, 0});

                  p->addActionLocation(
                      actions[i].name,
                      std::make_pair(actions[i].name[9] - '1', actions[i].name[12] - '1'));
              }
            });


            Solver solver;
            std::shared_ptr<Solution> solution = solver.solve(problem);
            // Save problem
            solver.writeSolution("tests/data/p1", solution);
        }

        /**
         * 3 robots start at the target and there are 3 boxes at the source to bring to the target
         *
         * \note Tests move to action
         */
        TEST(Problem, p2)
        {
            Problem problem;
            problem.init("tests/data/p2/domain.pddl", "tests/data/p2/problem.pddl", "tests/data/p2/parameters.json");

            problem.configureActions([](const std::vector<SASAction>& actions, Problem* p)
                                     {
                // All Actions have the same requirements
                for(unsigned int i = 0; i < actions.size(); ++i)
                {
                    p->actionToRequirements[actions[i].name] = i;
                    p->actionRequirements.push_back({0.25, 0, 0});
                    p->actionNonCumRequirements.push_back({0, 0, 0});

                    p->addActionLocation(
                        actions[i].name,
                        std::make_pair(actions[i].name[9] - '1', actions[i].name[12] - '1'));
                }
            });


            Solver solver;
            std::shared_ptr<Solution> solution = solver.solve(problem);

            // Save problem
            solver.writeSolution("tests/data/p2", solution);
        }

        /**
         * 3 robots start at the target and there are 3 boxes at the source to bring to the target
         * different robot speeds
         *
         * \note Tests hetero
         */
        TEST(Problem, p3)
        {
            Problem problem;
            problem.init("tests/data/p3/domain.pddl", "tests/data/p3/problem.pddl", "tests/data/p3/parameters.json");

            problem.configureActions([](const std::vector<SASAction>& actions, Problem* p)
                                     {
                // All Actions have the same requirements
                for(unsigned int i = 0; i < actions.size(); ++i)
                {
                    p->actionToRequirements[actions[i].name] = i;
                    p->actionRequirements.push_back({0.25, 0, 0});
                    p->actionNonCumRequirements.push_back({0, 0, 0});

                    p->addActionLocation(
                        actions[i].name,
                        std::make_pair(actions[i].name[9] - '1', actions[i].name[12] - '1'));
                }
            });

            Solver solver;
            std::shared_ptr<Solution> solution = solver.solve(problem);

            // Save problem
            solver.writeSolution("tests/data/p3", solution);
        }

        /**
         * 3 robots start at the target and there are 6 boxes at the source to bring to the target
         *
         * \note Tests reassigning
         */
        TEST(Problem, p4)
        {
            Problem problem;
            problem.init("tests/data/p4/domain.pddl", "tests/data/p4/problem.pddl", "tests/data/p4/parameters.json");

            problem.configureActions([](const std::vector<SASAction>& actions, Problem* p)
                                     {
                // All Actions have the same requirements
                for(unsigned int i = 0; i < actions.size(); ++i)
                {
                    p->actionToRequirements[actions[i].name] = i;
                    p->actionRequirements.push_back({0.25, 0, 0});
                    p->actionNonCumRequirements.push_back({0, 0, 0});

                    p->addActionLocation(
                        actions[i].name,
                        std::make_pair(actions[i].name[9] - '1', actions[i].name[12] - '1'));
                }
            });

            Solver solver;
            std::shared_ptr<Solution> solution = solver.solve(problem);

            // Save problem
            solver.writeSolution("tests/data/p4", solution);
        }

        /**
         * 3 robots start at the target and there are 6 boxes at the source to bring to the target
         * Different speeds
         *
         * \note Tests more complex reassigning (hetero)
         */
        TEST(Problem, p5)
        {
            Problem problem;
            problem.init("tests/data/p5/domain.pddl", "tests/data/p5/problem.pddl", "tests/data/p5/parameters.json");

            problem.configureActions([](const std::vector<SASAction>& actions, Problem* p)
                                     {
                // All Actions have the same requirements
                for(unsigned int i = 0; i < actions.size(); ++i)
                {
                    p->actionToRequirements[actions[i].name] = i;
                    p->actionRequirements.push_back({0.25, 0, 0});
                    p->actionNonCumRequirements.push_back({0, 0, 0});

                    p->addActionLocation(
                        actions[i].name,
                        std::make_pair(actions[i].name[9] - '1', actions[i].name[12] - '1'));
                }
            });

            Solver solver;
            std::shared_ptr<Solution> solution = solver.solve(problem);

            // Save problem
            solver.writeSolution("tests/data/p5", solution);
        }

        /**
         * 3 robots start at the target and there are 3 boxes at the source to bring to the target
         * boxes require 2 robots to carry
         *
         * \note Tests collab
         */
        TEST(Problem, p6)
        {
            Problem problem;
            problem.init("tests/data/p6/domain.pddl", "tests/data/p6/problem.pddl", "tests/data/p6/parameters.json");

            problem.configureActions([](const std::vector<SASAction>& actions, Problem* p)
                                     {
                // All Actions have the same requirements
                for(unsigned int i = 0; i < actions.size(); ++i)
                {
                    p->actionToRequirements[actions[i].name] = i;
                    p->actionRequirements.push_back({0.4, 0, 0});
                    p->actionNonCumRequirements.push_back({0, 0, 0});

                    p->addActionLocation(
                        actions[i].name,
                        std::make_pair(actions[i].name[9] - '1', actions[i].name[12] - '1'));
                }
            });

            Solver solver;
            std::shared_ptr<Solution> solution = solver.solve(problem);

            // Save problem
            solver.writeSolution("tests/data/p6", solution);
        }

        /**
         * 2 ground robots start at the start and there are 2 boxes. There is an obstacle in the middle
         *
         * \note Tests the MP is running
         */
        TEST(Problem, p7)
        {
            Problem problem;
            problem.init("tests/data/p7/domain.pddl", "tests/data/p7/problem.pddl", "tests/data/p7/parameters.json");

            problem.configureActions([](const std::vector<SASAction>& actions, Problem* p)
                                     {
                                       // All Actions have the same requirements
                                       for(unsigned int i = 0; i < actions.size(); ++i)
                                       {
                                           p->actionToRequirements[actions[i].name] = i;
                                           p->actionRequirements.push_back({0.2, 0, 0});
                                           p->actionNonCumRequirements.push_back({0, 0, 0});

                                           p->addActionLocation(
                                               actions[i].name,
                                               std::make_pair(actions[i].name[9] - '1', actions[i].name[12] - '1'));
                                       }
                                     });

            Solver solver;
            std::shared_ptr<Solution> solution = solver.solve(problem);

            // Save problem
            solver.writeSolution("tests/data/p7", solution);
        }

        /**
         * 1 ground robot and 1 aerial robot start at the start and there are 2 boxes. There is a ground obstacle in the middle
         *
         * \note Tests that separate MPs are running
         */
        TEST(Problem, p8)
        {
            Problem problem;
            problem.init("tests/data/p8/domain.pddl", "tests/data/p8/problem.pddl", "tests/data/p8/parameters.json");

            problem.configureActions([](const std::vector<SASAction>& actions, Problem* p)
                                     {
                                       // All Actions have the same requirements
                                       for(unsigned int i = 0; i < actions.size(); ++i)
                                       {
                                           p->actionToRequirements[actions[i].name] = i;
                                           p->actionRequirements.push_back({0.2, 0, 0});
                                           p->actionNonCumRequirements.push_back({0, 0, 0});

                                           p->addActionLocation(
                                               actions[i].name,
                                               std::make_pair(actions[i].name[9] - '1', actions[i].name[12] - '1'));
                                       }
                                     });

            Solver solver;
            std::shared_ptr<Solution> solution = solver.solve(problem);

            // Save problem
            solver.writeSolution("tests/data/p8", solution);
        }

        /**
         * 2 ground robots and 2 aerial robots start at the start and there are 2 boxes that require 2 robots to carry. No obstacles
         * There are 2 ground-aerial pairs in terms of speed
         *
         * \note Should test that ground robots can only pair with ground robots and aerials with aerials
         */
        TEST(Problem, p9)
        {
            Problem problem;
            problem.init("tests/data/p9/domain.pddl", "tests/data/p9/problem.pddl", "tests/data/p9/parameters.json");

            problem.configureActions([](const std::vector<SASAction>& actions, Problem* p)
                                     {
                                       // All Actions have the same requirements
                                       for(unsigned int i = 0; i < actions.size(); ++i)
                                       {
                                           p->actionToRequirements[actions[i].name] = i;
                                           p->actionRequirements.push_back({0.4, 0, 0});
                                           p->actionNonCumRequirements.push_back({0, 0, 0});

                                           p->addActionLocation(
                                               actions[i].name,
                                               std::make_pair(actions[i].name[9] - '1', actions[i].name[12] - '1'));
                                       }
                                     });

            Solver solver;
            std::shared_ptr<Solution> solution = solver.solve(problem);

            // Save problem
            solver.writeSolution("tests/data/p9", solution);
        }

        /**
         * 2 ground robots and 2 aerial robots that start at various starting locations, there are 4 boxes needing moving.
         * Ground robots can pick up a box by themselves, but aerials need 2 robots. Aerials are faster though. Obstacle
         * in the middle for both but smaller for aerials.
         *
         * \note Should test that ground robots can only pair with ground robots and aerials with aerials
         */
        TEST(Problem, p10)
        {
            Problem problem;
            problem.init("tests/data/p10/domain.pddl", "tests/data/p10/problem.pddl", "tests/data/p10/parameters.json");

            std::map<std::string, unsigned int> location_name_to_index{
                {"gs", 0},
                {"as", 1},
                {"bs", 2},
                {"bt", 3}
            };

            problem.configureActions([location_name_to_index](const std::vector<SASAction>& actions, Problem* p)
                                     {
                                       // All Actions have the same requirements
                                       for(unsigned int i = 0; i < actions.size(); ++i)
                                       {
                                           p->actionToRequirements[actions[i].name] = i;
                                           p->actionRequirements.push_back({0.4, 0, 0});
                                           p->actionNonCumRequirements.push_back({0, 0, 0});

                                           const unsigned int action_start = location_name_to_index.at(actions[i].name.substr(8, 2));
                                           const unsigned int action_end = location_name_to_index.at(actions[i].name.substr(11, 2));

                                           p->addActionLocation(
                                               actions[i].name,
                                               std::make_pair(action_start, action_end));
                                       }
                                     });

            Solver solver;
            std::shared_ptr<Solution> solution = solver.solve(problem);

            // Save problem
            solver.writeSolution("tests/data/p10", solution);
        }
    }  // namespace test
}  // namespace grstaps
