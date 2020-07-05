#ifndef TABU_CPP
#define TABU_CPP


#include <grstaps/Scheduling/tabu.h>


namespace grstaps
{
    class Scheduler;
    Scheduler tabu::solve(int numCandidate, Scheduler& initialSched)
    {
        optimal = initialSched.getMakeSpan();
        bestSolutionScore = std::numeric_limits<double>::max();
        for(int loopCount = 0; loopCount < numCandidate; loopCount++){

            tabu_list.clear();
            int countTime = 0;
            initialSched.getRandomDisjunct(currentSched);
            double score;
            bestSolverScore = std::numeric_limits<double>::max();

            if(bestSolutionScore <= (QUALITY_THRESHOLD * optimal))
            {
                break;
            }
            for(int i = 0; i < NUM_INTERATION; i++){

                this->getBestNearbySolution(i);
                score = currentSched.getMakeSpan();

                if(score < bestSolverScore){

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

    void tabu::getBestNearbySolution(int it)
    {
        double bestScore= std::numeric_limits<double>::max();
        int bestDisSwitch = -1;
        std::string currentDisID = currentSched.getDisjuctiveID();
        std::string id;

        for(int i = 0; i < currentSched.getDisjuctiveSize(); i++)
        {
            //Scheduler currentSchedCopy(currentSched);
            //float currentMakespan =  currentSchedCopy.getShedSwitch(i);
            if(currentDisID[i] == 1){
                id = currentDisID;
                id[i] = 0;
            }
            else{
                id = currentDisID;
                id[i] = 1;
            }
            float currentMakespan = -1;
            auto found = tabu_list.find(id);
            if(found == tabu_list.end()){
                    currentMakespan = currentSched.getShedSwitchTime(i);
            }
            else if(found->second <= it){
                currentMakespan = foundMakespans.find(id)->second;
                currentMakespan = found->second;
                currentMakespan = foundMakespans.at(id);
            }

            if(currentMakespan > 0)
            {
                if((bestScore > currentMakespan && found == tabu_list.end()) || (bestScore > currentMakespan && found->second <= it))
                {
                    bestDisSwitch = i;
                    bestScore     = currentMakespan;
                    tabu_list[id] = (it + TABU_LENGTH);
                    foundMakespans[id] = currentMakespan;
                }
            }
        }
        if(bestDisSwitch < 0)
            return;
        auto valid = currentSched.getShedSwitch(bestDisSwitch);
    }
}

/*
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
            if((bestScore > penalScore && tabu_list.find(id) == tabu_list.end() || bestScore > penalScore && tabu_list.find(id)->second <= it) || currentScore < bestSolverScore)
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
*/
#endif