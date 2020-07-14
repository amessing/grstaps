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
#include <memory>
#include <string>

// external
#include <nlohmann/json.hpp>

// local
#include "grstaps/problem.hpp"
#include "grstaps/solution.hpp"

namespace grstaps
{
    int main(int argc, char** argv)
    {

        // todo: Read config file
        nlohmann::json config;

        const int num_runs = config["num_runs"];
        const std::string solution_folder = config["solution_folder"];

        // todo: create folder based on time stamp in solution folder
        std::string timestamp;

        for(int i = 0; i < num_runs; ++i)
        {
            /*
            std::shared_ptr<Problem> problem = Problem::createSurvivorProblem(config["problem"]);
            // todo: so check to see if problem is solvable?
            problem.write(fmt::format("{0:s}/{1:s}/{2:d}.problem", solution_folder, timestamp, i));
            Solver solver(problem);
            std::shared_ptr<Solution> solution = solver.solve();
            solution.write(fmt::format("{0:s}/{1:s}/{2:d}.sol", solution_folder, timestamp, i));
            solver.wrtie(fmt::format("{0:s}/{1:s}/{2:d}.stats", solution_folder, timestamp, i));
            */
        }

        return 0;
    }
}


int main(int argc, char** argv)
{
    return grstaps::main(argc, argv);
}