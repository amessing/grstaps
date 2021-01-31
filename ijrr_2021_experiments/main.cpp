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
#include <grstaps/solver_sequential.hpp>
#include <grstaps/solution.hpp>

namespace grstaps
{
    void writeSolution(const std::string& folder, const std::string& filename, std::shared_ptr<Solution> solution)
    {
        if(!std::filesystem::exists(folder))
        {
            std::filesystem::create_directories(folder);
        }

        std::string filepath = fmt::format("{}/{}.json", folder, filename);
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
            args::Command single(group, "single", "single-threaded");
            args::Command sequential(group, "sequential", "sequential");
            args::Command fcpop(group, "fcpop", "fcpop");

            args::Group group2(sequential, "Sequential Types", args::Group::Validators::Xor);
            args::Flag tp_anytime(group2, "tp_anytime", "TP Anytime", {"tp_a"});
            args::Flag ta_anytime(group2, "ta_anytime", "TA Anytime", {"ta_a"});

            args::ValueFlag<float> ns_time(sequential, "ns_time", "Time taken by the non-sequential version", {"nst"});

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
                std::cout << "Single-threaded: problem " << problem_nr.Get() << " instance " << instance_nr.Get() << std::endl;
                Problem problem;
                std::string folder = fmt::format("problems/{0}/{1}", problem_nr.Get(), instance_nr.Get());
                problem.init(fmt::format("{0}/domain.pddl", folder).c_str(),
                             fmt::format("{0}/problem.pddl", folder).c_str(),
                             fmt::format("{0}/config.json", folder).c_str(),
                             fmt::format("{0}/map.json", folder).c_str());

                problem.writeMap(folder);

                SolverSingleThreaded solver;
                std::shared_ptr<Solution> solution = solver.solve(problem);
                writeSolution(folder, "st_output", solution);
            }
            else if(sequential)
            {
                std::cout << "Sequential: problem " << problem_nr.Get() << " instance " << instance_nr.Get() << std::endl;
                Problem problem;
                std::string folder = fmt::format("problems/{0}/{1}", problem_nr.Get(), instance_nr.Get());
                problem.init(fmt::format("{0}/domain.pddl", folder).c_str(),
                             fmt::format("{0}/problem.pddl", folder).c_str(),
                             fmt::format("{0}/config.json", folder).c_str(),
                             fmt::format("{0}/map.json", folder).c_str());

                problem.writeMap(folder);

                bool tp_anytime_val = tp_anytime;
                SolverSequential solver;
                std::shared_ptr<Solution> solution = solver.solve(problem, ns_time.Get(), tp_anytime_val);
                writeSolution(folder, "seq_output", solution);
            }
            else if(fcpop)
            {
                // TODO
            }

            return 0;
        }
    }
}

int main(int argc, char** argv)
{
    return grstaps::ijrr2021::main(argc, argv);
}