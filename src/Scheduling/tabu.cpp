#ifndef TABU_CPP
#define TABU_CPP


#include <grstaps/Scheduling/tabu.h>


namespace grstaps
{
    class Scheduler;
    Scheduler tabu::solve(int numCandidate, Scheduler initialSched)
    {
        optimal = initialSched.getMakeSpan();
        bestSolutionScore = std::numeric_limits<double>::max();
        for(int loopCount = 0; loopCount < numCandidate; loopCount++)
        {
            tabu_list.clear();
            int countTime = 0;
            currentSched  = initialSched.getRandomDisjunct();
            double score;
            bestSolverScore = std::numeric_limits<double>::max();

            if(bestSolutionScore <= (QUALITY_THRESHOLD * optimal))
            {
                break;
            }
            for(int i = 0; i < NUM_INTERATION; i++)
            {
                this->getBestNearbySolution(i);
                score = currentSched.getMakeSpan();

                if(score < bestSolverScore)
                {
                    bestSolverScore = score;
                    countTime       = 0;
                    if(bestSolverScore < bestSolutionScore){
                        bestSolution      = currentSched;
                        bestSolutionScore = bestSolverScore;
                    }
                }
                else
                {
                    countTime++;
                    if(countTime > TIME_TRY || bestSolutionScore <= (QUALITY_THRESHOLD * optimal))
                    {
                        break;
                    }
                }
            }
        }
        return bestSolution;
    }

    Scheduler tabu::getBestNearbySolution(int it)
    {
        double bestScore         = std::numeric_limits<double>::max();
        Scheduler bestChildSched = currentSched;
        for(int i = 0; i < currentSched.getDisjuctiveSize(); i++)
        {
            Scheduler s = currentSched.getShedSwitch(i);
            if(s.scheduleValid)
            {
                double currentScore = s.getMakeSpan();
                std::string id      = s.getDisjuctiveID();
                double penalScore   = currentScore;
                if((bestScore > penalScore && tabu_list.find(id) != tabu_list.end() || bestScore > penalScore && tabu_list.find(id)->second <= it) || currentScore < bestSolverScore)
                {
                    bestChildSched = s;
                    bestScore      = penalScore;
                    tabu_list[id]  = (it + TABU_LENGTH);
                }
            }
        }
        return bestChildSched;
    }
}

#endif