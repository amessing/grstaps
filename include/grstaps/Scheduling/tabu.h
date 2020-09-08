#ifndef TABU
#define TABU

#include <string>
#include <iostream>
#include <../lib/unordered_map/robin_hood.h>
#include <grstaps/Scheduling/Scheduler.h>

#define TABU_LENGTH 200
#define NUM_INTERATION 20
#define TIME_TRY 50
#define QUALITY_THRESHOLD 1.5

namespace grstaps
{
    class tabu
    {
       public:
        Scheduler solve(int numCandidate, Scheduler& initialSolution);

        void getBestNearbySolution(int);

       private:
        double bestSolverScore;
        robin_hood::unordered_map<std::string, int> tabu_list;
        robin_hood::unordered_map<std::string, float> foundMakespans;
        Scheduler bestSolution;
        Scheduler currentSched;
        double bestSolutionScore;
        double optimal;
    };

}
#endif
