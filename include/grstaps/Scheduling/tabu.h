#ifndef TABU
#define TABU

#include <string>
#include <iostream>
#include <../lib/unordered_map/robin_hood.h>
#include <grstaps/Scheduling/Scheduler.h>

#define TABU_LENGTH 40
#define NUM_INTERATION 3000
#define TIME_TRY 500
#define QUALITY_THRESHOLD 1.1

namespace grstaps
{
    class tabu
    {
       public:
        Scheduler solve(int numCandidate, Scheduler initialSolution);

        Scheduler getBestNearbySolution(int);

       private:
        double bestSolverScore{};
        robin_hood::unordered_map<std::string, int> tabu_list;
        Scheduler bestSolution;
        Scheduler currentSched;
        double bestSolutionScore;
        double optimal;
    };

}
#endif
