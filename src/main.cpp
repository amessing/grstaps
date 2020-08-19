/*
 * Copyright (C) 2020 Andrew Messing
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

// global
#include <string>
#include <vector>

// external
#include <nlohmann/json.hpp>

// local
#include "grstaps/problem.hpp"
#include "grstaps/solution.hpp"
#include "grstaps/solver.hpp"
#include "grstaps/task_planning/planner_parameters.hpp"
#include "grstaps/task_planning/setup.hpp"

namespace grstaps
{
    int run(int argc, char** argv)
    {
        Problem problem;

        std::vector<Problem::TraitVector> robot_traits = {{0.25}, {0.25}, {0.25}};
        problem.setRobotTraitVector(robot_traits);

        char* domain_filename  = "tests/data/p1/domain.pddl";
        char* problem_filename = "tests/data/p1/problem.pddl";
        char* output_filename  = "tests/data/p1/output";

        PlannerParameters parameters;

        parameters.domainFileName         = domain_filename;
        parameters.problemFileName        = problem_filename;
        parameters.outputFileName         = output_filename;
        parameters.generateGroundedDomain = true;
        // parameters.generateMutexFile = true;
        // parameters.generateTrace = true;
        SASTask* task = Setup::doPreprocess(&parameters);

        // Do we still need these?
        task->computeInitialState();
        task->computeRequirers();
        task->computeProducers();
        task->computePermanentMutex();
        problem.setTask(task);

        // All Actions have the same requirements
        for(unsigned int i = 0; i < task->actions.size(); ++i)
        {
            problem.actionToRequirements[task->actions[i].name] = i;
            problem.actionRequirements.push_back({0.05});
            problem.actionNonCumRequirements.push_back({0});
        }

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
        int breakppoint = -1;
        // Save solution to file
    }
}  // namespace grstaps

int main(int argc, char** argv)
{
    return grstaps::run(argc, argv);
}
