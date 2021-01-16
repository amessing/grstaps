// Global
#include <signal.h>
#include <fstream>
#include <iostream>
#include <filesystem>

// External
#include <args.hxx>
#include <fmt/format.h>

#include <grstaps/problem.hpp>
#include <grstaps/solver_single_threaded.hpp>
#include <grstaps/solution.hpp>

namespace grstaps
{
    void writeSolution(const std::string& folder, std::shared_ptr<Solution> solution)
    {
        if(!std::filesystem::exists(folder))
        {
            std::filesystem::create_directories(folder);
        }

        std::string filepath = fmt::format("{}/output.json", folder);
        solution->write(filepath);
    }

    namespace ijrr2021
    {
        int main(int argc, char** argv)
        {
            args::ArgumentParser parser("IJRR Experiments");
            args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
            
            args::ValueFlag<int> problem_nr(parser, "problem_nr", "Problem Number", {'p'});
            args::ValueFlag<int> instance_nr(parser, "instance_nr", "Instance Number", {'i'});
            
            args::Group group(parser, "Execution configs", args::Group::Validators::Xor);
            args::Flag single(group, "single", "single-threaded", {"single"});
            args::Flag parallel(group, "parallel", "multi-threaded", {"multi"});
            args::Flag sequential(group, "sequential", "sequential", {"seq"});
            args::Flag fcpop(group, "fcpop", "fcpop", {"fcpop"});

            try
            {
                parser.ParseCLI(argc, argv);
            }
            catch (args::Help)
            {
                std::cout << parser;
                return 0;
            }
            catch (args::ParseError e)
            {
                std::cerr << e.what() << std::endl;
                std::cerr << parser;
                return 1;
            }
            catch (args::ValidationError e)
            {
                std::cerr << e.what() << std::endl;
                std::cerr << parser;
                return 1;
            }



            if(single)
            {
                Problem problem;
                std::string folder = fmt::format("problems/{0}/{1}", problem_nr.Get(), instance_nr.Get());
                problem.init(fmt::format("{0}/domain.pddl", folder).c_str(),
                             fmt::format("{0}/problem.pddl", folder).c_str(),
                             fmt::format("{0}/config.json", folder).c_str(),
                             fmt::format("{0}/map.json", folder).c_str());

                problem.writeMap(folder);

                SolverSingleThreaded solver;
                std::shared_ptr<Solution> solution = solver.solve(problem);
                writeSolution(folder, solution);
            }
            else if(parallel)
            {

            }
            else if(sequential)
            {

            }
            else if(fcpop)
            {
                
            }


            return 0;
        }
    }
}

int main(int argc, char** argv)
{
    return grstaps::ijrr2021::main(argc, argv);
}