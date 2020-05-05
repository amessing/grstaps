//
// Created by glen on 4/5/20.
//

#include "grstaps/Scheduling/Scheduler.h"
#include <climits>
#include <ostream>
#include <../lib/unordered_map/robin_hood.h>
#include <vector>

    float Scheduler::initSTN(std::vector<float> durations){
    for(int i=0; i < durations.size(); ++i ){
        stn.push_back({0, durations[i]});
    }
};

    bool Scheduler::schedule( std::vector<float>* durations, std::vector<std::vector<int>>* orderingConstraints){
        scheduleValid = 0;
        initSTN(*durations);
        beforeConstraints = std::vector<std::vector<int>>(durations->size(), std::vector<int>(0));
        afterConstraints = std::vector<std::vector<int>>(durations->size(), std::vector<int>(0));
        for(int i=0; i < orderingConstraints->size(); ++i){
            bool added = addOC((*orderingConstraints)[i][0],(*orderingConstraints)[i][1]);
            if( added == 0){
                return 0;
            }
        }
        scheduleValid = 1;
        return 1;
    }

    float Scheduler::getMakeSpan(){
        if(scheduleValid){
            float max = 0;
            for(int i = 0; i < stn.size(); ++i)
            {
                if(max < stn[i][1])
                {
                    max = stn[i][1];
                }
            }
            return max;
        }
        else{
            return -1;
        }
    }

    bool Scheduler::addOC(int first, int second) {
        beforeConstraints[first].push_back(second);
        afterConstraints[second].push_back(first);

        std::vector<std::vector<int>> constraintsToUpdate;
        constraintsToUpdate.push_back(std::vector<int>{first, second});

        std::vector<std::vector<bool>> checkedAlready(int(stn.size()), std::vector<bool>(stn.size(), 0));
        checkedAlready[first][second] = 1;
        while(constraintsToUpdate.size() > 0){
            if( stn[second][0] <  stn[first][1]){
                float duration = stn[second][1] - stn[second][0];
                stn[second][0] = stn[first][1];
                stn[second][1] = stn[second][0] + duration;
                for(int i = 0; i < beforeConstraints[second].size(); ++i)
                {
                    constraintsToUpdate.push_back(std::vector<int>{second, beforeConstraints[second][i]});
                }
            }
            checkedAlready[first][second] = 1;
            constraintsToUpdate.erase(constraintsToUpdate.begin());
            if( constraintsToUpdate.size() != 0)
            {
                first  = constraintsToUpdate[0][0];
                second = constraintsToUpdate[0][1];
                if(checkedAlready[first][second] == 1)
                {
                    std::string errorString = "Schedule invalid loop found";
                    std::cout << errorString << std::endl;
                    return 0;
                }
            }

        }
        return 1;
    }
    
    bool Scheduler::increaseActionTime(int actionIndex, int duration){
        stn[actionIndex][1] = stn[actionIndex][0] + duration;
        std::vector<std::vector<int>> constraintsToUpdate;
        std::vector<std::vector<bool>> checkedAlready(int(stn.size()), std::vector<bool>(stn.size(), 0));
        int second = actionIndex;
        int first = actionIndex;

        for(int i = 0; i < beforeConstraints[second].size(); ++i){
            constraintsToUpdate.push_back(std::vector<int>{second, beforeConstraints[second][i]});
        }
        if( constraintsToUpdate.size() > 0){
            first  = constraintsToUpdate[0][0];
            second = constraintsToUpdate[0][1];
        }

        while(constraintsToUpdate.size() > 0){
            if( stn[second][0] <  stn[first][1]){
                float duration = stn[second][1] - stn[second][0];
                stn[second][0] = stn[first][1];
                stn[second][1] = stn[second][0] + duration;
                for(int i = 0; i < beforeConstraints[second].size(); ++i)
                {
                    constraintsToUpdate.push_back(std::vector<int>{second, beforeConstraints[second][i]});
                }
            }
            checkedAlready[first][second] = 1;
            constraintsToUpdate.erase(constraintsToUpdate.begin());
            if( constraintsToUpdate.size() != 0)
            {
                first  = constraintsToUpdate[0][0];
                second = constraintsToUpdate[0][1];
                if(checkedAlready[first][second] == 1)
                {
                    std::string errorString = "Schedule invalid loop found";
                    std::cout << errorString << std::endl;
                    return 0;
                }
            }

        }
        return 1;
    }

    bool Scheduler::decreaseActionTime(int actionIndex, int duration){
        stn[actionIndex][1] = duration + stn[actionIndex][0];

        std::vector<std::vector<int>> constraintsToUpdate;
        std::vector<std::vector<bool>> checkedAlready(int(stn.size()), std::vector<bool>(stn.size(), 0));
        for(int i = 0; i < beforeConstraints[actionIndex].size(); ++i){
            constraintsToUpdate.push_back(std::vector<int>{actionIndex, beforeConstraints[actionIndex][i]});
        }

        int first = 0;
        int second = 0;
        if( constraintsToUpdate.size() > 0){
            first  = constraintsToUpdate[0][0];
            second = constraintsToUpdate[0][1];
        }

        while(constraintsToUpdate.size() > 0){
            float maxDelay = 0;
            for(int i = 0; i < afterConstraints[second].size(); ++i){
                if(stn[afterConstraints[second][i]][1] > maxDelay){
                    maxDelay = stn[afterConstraints[second][i]][1];
                }
            }
            if( stn[second][0] != maxDelay){
                float duration = stn[second][1] - stn[second][0];
                stn[second][0] = maxDelay;
                stn[second][1] = duration;
                for(int i = 0; i < beforeConstraints[second].size(); ++i){
                    constraintsToUpdate.push_back(std::vector<int>{second, beforeConstraints[second][i]});
                }
            }

            checkedAlready[first][second] = 1;

            constraintsToUpdate.erase(constraintsToUpdate.begin());
            if( constraintsToUpdate.size() != 0){
                first  = constraintsToUpdate[0][0];
                second = constraintsToUpdate[0][1];
                if(checkedAlready[first][second] == 1){
                    std::string errorString = "Schedule invalid loop found";
                    std::cout << errorString << std::endl;
                    return 0;
                }
            }
        }
    return 1;
    }

    bool Scheduler::removeOC(int first, int second){
        beforeConstraints[first].erase(std::remove(beforeConstraints[first].begin(), beforeConstraints[first].end(), second), beforeConstraints[first].end());
        afterConstraints[second].erase(std::remove(afterConstraints[second].begin(), afterConstraints[second].end(), first), afterConstraints[second].end());

        if(stn[first][1] == stn[second][0])
        {
            float maxDelay = 0;
            for(int i = 0; i < afterConstraints[second].size(); ++i)
            {
                if(stn[afterConstraints[second][i]][1] > maxDelay)
                {
                    maxDelay = stn[afterConstraints[second][i]][1];
                }
            }
            float duration = stn[second][1] - stn[second][0];
            stn[second][0] = maxDelay;
            stn[second][1] = duration + stn[second][0];

            std::vector<std::vector<int>> constraintsToUpdate;
            std::vector<std::vector<bool>> checkedAlready(int(stn.size()), std::vector<bool>(stn.size(), 0));
            for(int i = 0; i < beforeConstraints[second].size(); ++i){
                constraintsToUpdate.push_back(std::vector<int>{second, beforeConstraints[second][i]});
            }

            if( constraintsToUpdate.size() > 0){
                first  = constraintsToUpdate[0][0];
                second = constraintsToUpdate[0][1];
            }

            while(constraintsToUpdate.size() > 0){

                maxDelay = 0;
                for(int i = 0; i < afterConstraints[second].size(); ++i){
                    if(stn[afterConstraints[second][i]][1] > maxDelay){
                        maxDelay = stn[afterConstraints[second][i]][1];
                    }
                }
                if( stn[second][0] != maxDelay){
                    float duration = stn[second][1] - stn[second][0];
                    stn[second][0] = maxDelay;
                    stn[second][1] = duration;
                    for(int i = 0; i < beforeConstraints[second].size(); ++i){
                        constraintsToUpdate.push_back(std::vector<int>{second, beforeConstraints[second][i]});
                    }
                }

                checkedAlready[first][second] = 1;

                constraintsToUpdate.erase(constraintsToUpdate.begin());
                if( constraintsToUpdate.size() != 0){
                    first  = constraintsToUpdate[0][0];
                    second = constraintsToUpdate[0][1];
                    if(checkedAlready[first][second] == 1){
                        std::string errorString = "Schedule invalid loop found";
                        std::cout << errorString << std::endl;
                        return 0;
                    }
                }
            }
        }
        return 1;
    }

    bool Scheduler::removeAction(int actionID){
        while(beforeConstraints[actionID].size() > 0){
            Scheduler::removeOC(actionID, beforeConstraints[actionID][0]);
        }
        for(int i=0; i < afterConstraints[actionID].size(); ++i){
            beforeConstraints[afterConstraints[actionID][i]].erase(std::remove(beforeConstraints[afterConstraints[actionID][i]].begin(), beforeConstraints[afterConstraints[actionID][i]].end(), actionID), beforeConstraints[afterConstraints[actionID][i]].end());
        }
        stn.erase(stn.begin() + actionID);


    }

    bool Scheduler::addAction(float duration, std::vector<int> orderingConstraints){
        stn.push_back(std::vector<float>{0, duration});
        for(int i=0; i < orderingConstraints.size(); ++i){
            addOC(stn.size()-1, orderingConstraints[i]);
        }
    }

