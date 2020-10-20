// Global
#include <signal.h>
#include <fstream>

// External
#include <args.hxx>

// GRSTAPS
#include <grstaps/logger.hpp>

// Local
#include "experiments.hpp"

int main(int argc, char** argv)
{
    args::ArgumentParser parser("ICRA Experiments");
    args::ValueFlag<float> alpha(parser, "alpha", "Hyperparameter", {'a'});
    args::ValueFlag<int> problem_nr(parser, "problem", "Problem Number", {'p'});
    args::ValueFlag<int> problem_ver(parser, "version", "Problem Version", {'v'});
    args::Flag seq(parser, "sequential", "Flag for running sequential", {'s'});
    try
    {
        parser.ParseCLI(argc, argv);
    }
    catch(args::ParseError e)
    {
        grstaps::Logger::error("{}\n{}", e.what(), parser);
        return 1;
    }
    catch(args::ValidationError e)
    {
        grstaps::Logger::error("{}\n{}", e.what(), parser);
        return 1;
    }

    float a;
    if(alpha)
    {
        a = args::get(alpha);
    }

    int pnr;
    if(problem_nr)
    {
        pnr = args::get(problem_nr);
    }

    int pv = 0;
    if(problem_ver)
    {
        pv = args::get(problem_ver);
    }
    else
    {
        throw "problem version not set";
    }

    // Catch termination signal from timeout
    signal(SIGTERM, grstaps::icra2021::Experiments::handleSignal);
    signal(SIGKILL, grstaps::icra2021::Experiments::handleSignal);

    const std::string problems_folder = "problems";
    if(!std::experimental::filesystem::exists(problems_folder))
    {
        std::experimental::filesystem::create_directories(problems_folder);
    }

    const std::string outputs_folder = "outputs";
    if(!std::experimental::filesystem::exists(outputs_folder))
    {
        std::experimental::filesystem::create_directories(outputs_folder);
    }

    auto& experiments = grstaps::icra2021::Experiments::getInstance();
    experiments.setProblemVersion(pv);
    char* free_block = new char[32];
    nlohmann::json metrics;
    try
    {
        metrics = seq ? experiments.solveSequentially(pnr) : experiments.solve(pnr, a);
        //grstaps::Logger::debug("metrics object size {}", sizeof(metrics));
    }
    catch(std::bad_alloc&)
    {
        delete free_block;
        metrics = experiments.errorData();
    }
    experiments.clear();


    std::ofstream out(fmt::format("outputs/output_{0:d}_{1:0.2f}_{2:d}.json", pnr, alpha ? a : 0.0f, seq ? 1 : 0));
    out << metrics;
    out.flush();

    return 0;
}