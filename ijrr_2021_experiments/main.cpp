// Global
#include <signal.h>
#include <fstream>
#include <iostream>
#include <filesystem>

// External
#include <args.hxx>
#include <fmt/format.h>

#include <grstaps/problem.hpp>
#include <grstaps/solver_fcpop.hpp>
#include <grstaps/solver_single_threaded.hpp>
#include <grstaps/solver_sequential.hpp>
#include <grstaps/solution.hpp>
#include <grstaps/task_planning/planner_parameters.hpp>
#include <grstaps/task_planning/setup.hpp>

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

            args::ValueFlag<std::string> ext(parser, "ext", "Folder Extension", {'e'});
            args::ValueFlag<std::string> problem_nr(parser, "problem_nr", "Problem Number", {'p'});
            args::ValueFlag<int> instance_nr(parser, "instance_nr", "Instance Number", {'i'});
            
            args::Group group(parser, "Execution configs", args::Group::Validators::Xor);
            args::Command single(group, "single", "single-threaded");
            args::Command sequential(group, "sequential", "sequential");
            args::Command fcpop(group, "fcpop", "fcpop");
            args::Command fcpop_ga(group, "fcpop_ga", "fcpop_ga");

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
                std::string folder = fmt::format("problems{2}/{0}/{1}", problem_nr.Get(), instance_nr.Get(), ext.Get());
                problem.init(fmt::format("{0}/domain_grstaps.pddl", folder).c_str(),
                             fmt::format("{0}/problem_grstaps.pddl", folder).c_str(),
                             fmt::format("{0}/config.json", folder).c_str(),
                             fmt::format("{0}/map.json", folder).c_str());

                problem.writeMap(folder);

                SolverSingleThreaded solver;
                std::shared_ptr<Solution> solution = solver.solve(problem);
                writeSolution(folder, "st_output", solution);
            }
            else if(sequential)
            {
                std::cout << "Sequential: problem " << problem_nr.Get() << " instance " << instance_nr.Get() << " timeout " << ns_time.Get() << std::endl;
                Problem problem;
                std::string folder = fmt::format("problems{2}/{0}/{1}", problem_nr.Get(), instance_nr.Get(), ext.Get());
                problem.init(fmt::format("{0}/domain.pddl", folder).c_str(),
                             fmt::format("{0}/problem.pddl", folder).c_str(),
                             fmt::format("{0}/config.json", folder).c_str(),
                             fmt::format("{0}/map.json", folder).c_str());

                problem.writeMap(folder);

                bool tp_anytime_val = tp_anytime;
                SolverSequential solver;
                std::shared_ptr<Solution> solution = solver.solve(problem, ns_time.Get(), tp_anytime_val);
                if(tp_anytime_val)
                {
                    writeSolution(folder, "seq_tp_output", solution);
                }
                else
                {
                    writeSolution(folder, "seq_ta_output", solution);
                }
            }
            else if(fcpop)
            {
                std::cout << "Fcpop: problem " << problem_nr.Get() << " instance " << instance_nr.Get() << std::endl;
                std::string folder = fmt::format("problems{2}/{0}/{1}", problem_nr.Get(), instance_nr.Get(), ext.Get());
                SolverFcpop solver;
                nlohmann::json fcpop_output = solver.solve(fmt::format("{0}/domain_fcpop.pddl", folder),
                                                           fmt::format("{0}/problem_fcpop.pddl", folder));
                if(!std::filesystem::exists(folder))
                {
                    std::filesystem::create_directories(folder);
                }

                std::string filepath = fmt::format("{}/{}.json", folder, "fcpop_output");
                std::ofstream output;
                output.open(filepath.c_str());
                output << fcpop_output.dump(4);
                output.close();
            }
            else if(fcpop_ga)
            {
                PlannerParameters parameters;
                std::string folder = fmt::format("problems{2}/{0}/{1}", problem_nr.Get(), instance_nr.Get(), ext.Get());
                std::string domain_filename = fmt::format("{0}/domain_fcpop.pddl", folder);
                parameters.domainFileName  = domain_filename.c_str();
                std::string problem_filename = fmt::format("{0}/problem_fcpop.pddl", folder);
                parameters.problemFileName = problem_filename.c_str();
                auto task = Setup::doPreprocess(&parameters);
                nlohmann::json metrics = {
                    {"total_grounded_actions", task->actions.size()}
                };
                std::string filepath = fmt::format("{}/{}.json", folder, "fcpop_ga");
                std::ofstream output;
                output.open(filepath.c_str());
                output << metrics.dump(4);
                output.close();
            }

            return 0;
        }
    }
}

int main(int argc, char** argv)
{
    return grstaps::ijrr2021::main(argc, argv);
}