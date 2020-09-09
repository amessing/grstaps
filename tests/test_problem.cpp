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

#include <nlohmann/json.hpp>

// local
#include <grstaps/location.hpp>
#include <grstaps/problem.hpp>
#include <grstaps/solver.hpp>
#include <grstaps/task_planning/planner_parameters.hpp>
#include <grstaps/task_planning/sas_task.hpp>
#include <grstaps/task_planning/setup.hpp>

namespace grstaps
{
    namespace test
    {
        /**
         * 3 robots start at the source with 3 boxes to bring to the target
         */
        TEST(Problem, p1)
        {
            Problem problem;

            PlannerParameters parameters;
            char* domain_filename  = "tests/data/p1/domain.pddl";
            char* problem_filename = "tests/data/p1/problem.pddl";
            parameters.domainFileName  = domain_filename;
            parameters.problemFileName = problem_filename;
            SASTask* task = Setup::doPreprocess(&parameters);
            problem.setTask(task);

            std::ifstream ifs("tests/data/p1/parameters.json");
            nlohmann::json config;
            ifs >> config;

            problem.setLocations(config["locations"]);
            problem.setStartingLocations(config["starting_locations"]);

            problem.setRobotTraitVector(config["robot_traits"]);
            problem.speedIndex = config["speed_index"];

            // All Actions have the same requirements
            for(unsigned int i = 0; i < task->actions.size(); ++i)
            {
                problem.actionToRequirements[task->actions[i].name] = i;
                problem.actionRequirements.push_back({0.25, 0});
                problem.actionNonCumRequirements.push_back({0, 0});

                problem.addActionLocation(
                    task->actions[i].name,
                    std::make_pair(task->actions[i].name[9] - '1', task->actions[i].name[12] - '1'));
            }

            problem.setConfig(config);

            Solver solver;
            std::shared_ptr<Solution> solution = solver.solve(problem);

            // Save problem
            solver.writeSolution("outputs/p1", solution);
        }

        /**
         * 3 robots start at the target and there are 3 boxes at the source to bring to the target
         */
        TEST(Problem, p2)
        {
            Problem problem;

            PlannerParameters parameters;
            char* domain_filename  = "tests/data/p2/domain.pddl";
            char* problem_filename = "tests/data/p2/problem.pddl";
            parameters.domainFileName  = domain_filename;
            parameters.problemFileName = problem_filename;
            SASTask* task = Setup::doPreprocess(&parameters);
            problem.setTask(task);

            std::ifstream ifs("tests/data/p2/parameters.json");
            nlohmann::json config;
            ifs >> config;

            problem.setLocations(config["locations"]);
            problem.setStartingLocations(config["starting_locations"]);

            problem.setRobotTraitVector(config["robot_traits"]);
            problem.speedIndex = config["speed_index"];

            // All Actions have the same requirements
            for(unsigned int i = 0; i < task->actions.size(); ++i)
            {
                problem.actionToRequirements[task->actions[i].name] = i;
                problem.actionRequirements.push_back({0.25,0});
                problem.actionNonCumRequirements.push_back({0,0});

                problem.addActionLocation(
                    task->actions[i].name,
                    std::make_pair(task->actions[i].name[9] - '1', task->actions[i].name[12] - '1'));
            }

            problem.setConfig(config);

            Solver solver;
            std::shared_ptr<Solution> solution = solver.solve(problem);

            // Save problem
            solver.writeSolution("outputs/p2", solution);
        }

        /**
         * 3 robots start at the target and there are 3 boxes at the source to bring to the target
         * different robot speeds
         */
        TEST(Problem, p3)
        {
            Problem problem;

            PlannerParameters parameters;
            char* domain_filename  = "tests/data/p3/domain.pddl";
            char* problem_filename = "tests/data/p3/problem.pddl";
            parameters.domainFileName  = domain_filename;
            parameters.problemFileName = problem_filename;
            SASTask* task = Setup::doPreprocess(&parameters);
            problem.setTask(task);

            std::ifstream ifs("tests/data/p3/parameters.json");
            nlohmann::json config;
            ifs >> config;

            problem.setLocations(config["locations"]);
            problem.setStartingLocations(config["starting_locations"]);

            problem.setRobotTraitVector(config["robot_traits"]);
            problem.speedIndex = config["speed_index"];

            // All Actions have the same requirements
            for(unsigned int i = 0; i < task->actions.size(); ++i)
            {
                problem.actionToRequirements[task->actions[i].name] = i;
                problem.actionRequirements.push_back({0.25,0});
                problem.actionNonCumRequirements.push_back({0,0});

                problem.addActionLocation(
                    task->actions[i].name,
                    std::make_pair(task->actions[i].name[9] - '1', task->actions[i].name[12] - '1'));
            }
            problem.setConfig(config);

            Solver solver;
            std::shared_ptr<Solution> solution = solver.solve(problem);

            // Save problem
            solver.writeSolution("outputs/p3", solution);
        }

        /**
         * 3 robots start at the target and there are 6 boxes at the source to bring to the target
         */
        TEST(Problem, p4)
        {
            Problem problem;

            PlannerParameters parameters;
            char* domain_filename  = "tests/data/p4/domain.pddl";
            char* problem_filename = "tests/data/p4/problem.pddl";
            parameters.domainFileName  = domain_filename;
            parameters.problemFileName = problem_filename;
            SASTask* task = Setup::doPreprocess(&parameters);
            problem.setTask(task);

            std::ifstream ifs("tests/data/p4/parameters.json");
            nlohmann::json config;
            ifs >> config;

            problem.setLocations(config["locations"]);
            problem.setStartingLocations(config["starting_locations"]);

            problem.setRobotTraitVector(config["robot_traits"]);
            problem.speedIndex = config["speed_index"];

            // All Actions have the same requirements
            for(unsigned int i = 0; i < task->actions.size(); ++i)
            {
                problem.actionToRequirements[task->actions[i].name] = i;
                problem.actionRequirements.push_back({0.25,0});
                problem.actionNonCumRequirements.push_back({0,0});

                problem.addActionLocation(
                    task->actions[i].name,
                    std::make_pair(task->actions[i].name[9] - '1', task->actions[i].name[12] - '1'));
            }
            problem.setConfig(config);

            Solver solver;
            std::shared_ptr<Solution> solution = solver.solve(problem);

            // Save problem
            solver.writeSolution("outputs/p4", solution);
        }

        /**
         * 3 robots start at the target and there are 6 boxes at the source to bring to the target
         * Different speeds
         */
        TEST(Problem, p5)
        {
            Problem problem;

            PlannerParameters parameters;
            char* domain_filename  = "tests/data/p5/domain.pddl";
            char* problem_filename = "tests/data/p5/problem.pddl";
            parameters.domainFileName  = domain_filename;
            parameters.problemFileName = problem_filename;
            SASTask* task = Setup::doPreprocess(&parameters);
            problem.setTask(task);

            std::ifstream ifs("tests/data/p5/parameters.json");
            nlohmann::json config;
            ifs >> config;

            problem.setLocations(config["locations"]);
            problem.setStartingLocations(config["starting_locations"]);

            problem.setRobotTraitVector(config["robot_traits"]);
            problem.speedIndex = config["speed_index"];

            // All Actions have the same requirements
            for(unsigned int i = 0; i < task->actions.size(); ++i)
            {
                problem.actionToRequirements[task->actions[i].name] = i;
                problem.actionRequirements.push_back({0.25, 0});
                problem.actionNonCumRequirements.push_back({0,0});

                problem.addActionLocation(
                    task->actions[i].name,
                    std::make_pair(task->actions[i].name[9] - '1', task->actions[i].name[12] - '1'));
            }
            problem.setConfig(config);

            Solver solver;
            std::shared_ptr<Solution> solution = solver.solve(problem);

            // Save problem
            solver.writeSolution("outputs/p5", solution);
        }

        /**
         * 3 robots start at the target and there are 3 boxes at the source to bring to the target
         * boxes require 2 robots to carry
         */
        TEST(Problem, p6)
        {
            Problem problem;

            PlannerParameters parameters;
            char* domain_filename  = "tests/data/p2/domain.pddl";
            char* problem_filename = "tests/data/p2/problem.pddl";
            parameters.domainFileName  = domain_filename;
            parameters.problemFileName = problem_filename;
            SASTask* task = Setup::doPreprocess(&parameters);
            problem.setTask(task);

            std::ifstream ifs("tests/data/p2/parameters.json");
            nlohmann::json config;
            ifs >> config;

            problem.setLocations(config["locations"]);
            problem.setStartingLocations(config["starting_locations"]);

            problem.setRobotTraitVector(config["robot_traits"]);
            problem.speedIndex = config["speed_index"];

            // All Actions have the same requirements
            for(unsigned int i = 0; i < task->actions.size(); ++i)
            {
                problem.actionToRequirements[task->actions[i].name] = i;
                problem.actionRequirements.push_back({0.4,0});
                problem.actionNonCumRequirements.push_back({0,0});

                problem.addActionLocation(
                    task->actions[i].name,
                    std::make_pair(task->actions[i].name[9] - '1', task->actions[i].name[12] - '1'));
            }
            problem.setConfig(config);

            Solver solver;
            std::shared_ptr<Solution> solution = solver.solve(problem);

            // Save problem
            solver.writeSolution("outputs/p6", solution);
        }
    }  // namespace test
}  // namespace grstaps
