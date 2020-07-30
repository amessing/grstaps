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
#include <grstaps/task_planning/sas_task.hpp>

namespace grstaps
{
    namespace test
    {
        TEST(Problem, simple)
        {
            Problem problem;

            std::vector<Location> locations = {Location("source", 0.5, 0.5), Location("target", 1.5, 1.5)};
            problem.setLocations(locations);

            std::vector<Problem::TraitVector> robot_traits = {{0.25}, {0.25}, {0.25}};
            problem.setRobotTraitVector(robot_traits);

            // Create task
            auto task = new SASTask;

            uint num_boxes = 3;

            // Variables
            std::vector<SASVariable*> vars;
            // Box
            // Location
            for(uint i = 0; i < num_boxes; ++i)
            {
                SASVariable* var = task->createNewVariable(fmt::format("box_{}_location", i));
                for(uint j = 0; j < locations.size(); ++j)
                {
                    var->addPossibleValue(j);
                }
                var->addInitialValue(0, true, 0.0);
                vars.push_back(var);
            }

            // Action
            // Move Box
            for(uint i = 0; i < num_boxes; ++i)
            {
                SASAction* action = task->createNewAction(fmt::format("move_box_{}", i));

                // Box 'i' starts at the source
                SASCondition condition(i, 0);
                action->startCond.push_back(condition);

                // Box 'i' ends at the target
                SASCondition effect(i, 1);
                action->endEff.push_back(effect);

                // Action takes 1 s
                SASDuration duration{'N', '=', {'N', 1.0}};
                action->duration.push_back(duration);
            }
            task->metric.type = 'T';  // makespan
            task->computeInitialState();
            task->computeRequirers();
            task->computeProducers();
            task->computePermanentMutex();
            problem.setTask(task);

            // No obstacles
            nlohmann::json config;

            // Config
            config["mp_boundary_min"] = 0;
            config["mp_boundary_max"] = 2;
            problem.setConfig(config);
            // Save problem

            Solver solver;
            std::shared_ptr<Solution> solution = solver.solve(problem);
            // Evaluate solution C++ exception with description "std::bad_alloc" thrown in the test body.

            // Save solution to file
        }
    }  // namespace test
}  // namespace grstaps
