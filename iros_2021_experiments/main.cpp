// Global
#include <signal.h>
#include <fstream>
#include <iostream>
#include <filesystem>

// External
#include <args.hxx>
#include <fmt/format.h>

#include "iros_problem.hpp"
#include "iros_solver.hpp"

namespace grstaps
{
    namespace iros2021
    {
        int main(int argc, char** argv)
        {
            args::ArgumentParser parser("IROS Experiments");
            args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
            args::ValueFlag<int> problem_nr(parser, "problem_nr", "Problem Number", {'p'});
            args::ValueFlag<std::string> map_file(parser, "map_file", "Map File", {'m'});

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

            std::cout << "ITAGS: problem " << problem_nr.Get() << std::endl;
            std::string folder = fmt::format("problems/{0}", problem_nr.Get());
            std::string config_filepath = fmt::format("{0}/config.json", folder);
            std::string map_filepath = fmt::format("maps/{0}", map_file.Get());
            IrosProblem problem;
            problem.init(config_filepath.c_str(), map_filepath.c_str());

            IrosSolver solver;
            nlohmann::json results = solver.solve(problem);
            std::ofstream output;
            output.open(fmt::format("{0}/itags_output.json", folder).c_str());
            output << results.dump(4);
            output.close();

            return 0;
        }
    }
}

int main(int argc, char** argv)
{
    return grstaps::iros2021::main(argc, argv);
}