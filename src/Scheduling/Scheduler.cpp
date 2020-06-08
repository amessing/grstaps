/*
 * Copyright (C)2020 Glen Neville
 *
 * GRSTAPS is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 3 of the License,
 * or any later version.
 *
 * GRSTAPS is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GRSTAPS; if not, write to the Free Software Foundation,
 * Inc., #59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <grstaps/Scheduling/Scheduler.h>
#include <ostream>
#include <utility>
#include <vector>
#include <numeric>


namespace grstaps {

    Scheduler::Scheduler(){
        scheduleValid = false;
        makeSpan = -1;
    }

    Scheduler::Scheduler(Scheduler& toCopy){
        stn = toCopy.stn;
        beforeConstraints = toCopy.beforeConstraints;
        afterConstraints = toCopy.afterConstraints;
        disjuctiveConstraints = toCopy.disjuctiveConstraints;
        disjuctiveOrderings = toCopy.disjuctiveOrderings;
        scheduleValid = toCopy.scheduleValid;
        makeSpan = toCopy.makeSpan;
        disID = toCopy.disID;
    }

    float Scheduler::initSTN(const std::vector<float>& durations){
        stn = std::vector<std::vector<float>>{};
        for(float duration : durations){
            stn.push_back({0, duration});
        }
        return 1;
    }

    float Scheduler::getMakeSpan(){
        if(scheduleValid){
            if(makeSpan >= 0){
                return makeSpan;
            }
            float max = 0;
            for(auto & i : stn){
                if(max < i[1]){
                    max = i[1];
                }
            }
            makeSpan= max;
            return max;
        }
        else{
            return -1;
        }
    }

    bool Scheduler::checkConcurrent(int first, int second) {
        return (stn[first][1] < stn[second][0] || stn[first][0] > stn[second][1]);
    }

    bool Scheduler::addOC(int first, int second) {
        beforeConstraints[first].push_back(second);
        afterConstraints[second].push_back(first);

        std::vector<std::vector<int>> constraintsToUpdate;
        constraintsToUpdate.push_back(std::vector<int>{first, second});

        std::vector<std::vector<bool>> checkedAlready(int(stn.size()), std::vector<bool>(stn.size(), false));
        checkedAlready[first][second] = true;
        while(!constraintsToUpdate.empty()){
            if( stn[second][0] <  stn[first][1]){
                float duration = stn[second][1] - stn[second][0];
                stn[second][0] = stn[first][1];
                stn[second][1] = stn[second][0] + duration;
                for(int i = 0; i < beforeConstraints[second].size(); ++i)
                {
                    constraintsToUpdate.push_back(std::vector<int>{second, beforeConstraints[second][i]});
                }
            }
            checkedAlready[first][second] = true;
            constraintsToUpdate.erase(constraintsToUpdate.begin());
            if( !constraintsToUpdate.empty())
            {
                first  = constraintsToUpdate[0][0];
                second = constraintsToUpdate[0][1];
                if(checkedAlready[first][second] == 1)
                {
                    //std::string errorString = "Schedule invalid loop found";
                    //std::cout << errorString << std::endl;
                    return false;
                }
            }

        }
        return true;
    }
    
    bool Scheduler::increaseActionTime(int actionIndex, float duration){
        stn[actionIndex][1] = stn[actionIndex][0] + duration;
        std::vector<std::vector<int>> constraintsToUpdate;
        std::vector<std::vector<bool>> checkedAlready(int(stn.size()), std::vector<bool>(stn.size(), false));
        int second = actionIndex;
        int first = actionIndex;

        constraintsToUpdate.reserve(beforeConstraints[second].size());
        for(int i = 0; i < beforeConstraints[second].size(); ++i){
            constraintsToUpdate.push_back(std::vector<int>{second, beforeConstraints[second][i]});
        }
        if( !constraintsToUpdate.empty()){
            first  = constraintsToUpdate[0][0];
            second = constraintsToUpdate[0][1];
        }

        while(!constraintsToUpdate.empty()){
            if( stn[second][0] <  stn[first][1]){
                float newDuration = stn[second][1] - stn[second][0];
                stn[second][0] = stn[first][1];
                stn[second][1] = stn[second][0] + newDuration;
                for(int i = 0; i < beforeConstraints[second].size(); ++i){
                    constraintsToUpdate.push_back(std::vector<int>{second, beforeConstraints[second][i]});
                }
            }
            checkedAlready[first][second] = true;
            constraintsToUpdate.erase(constraintsToUpdate.begin());
            if( !constraintsToUpdate.empty()){
                first  = constraintsToUpdate[0][0];
                second = constraintsToUpdate[0][1];
                if(checkedAlready[first][second] == 1)
                {
                    //std::string errorString = "Schedule invalid loop found";
                    //std::cout << errorString << std::endl;
                    return false;
                }
            }

        }
        return true;
    }

    bool Scheduler::decreaseActionTime(int actionIndex, float duration){
        stn[actionIndex][1] = duration + stn[actionIndex][0];

        std::vector<std::vector<int>> constraintsToUpdate;
        std::vector<std::vector<bool>> checkedAlready(int(stn.size()), std::vector<bool>(stn.size(), false));
        constraintsToUpdate.reserve(beforeConstraints[actionIndex].size());
        for(int i = 0; i < beforeConstraints[actionIndex].size(); ++i){
            constraintsToUpdate.push_back(std::vector<int>{actionIndex, beforeConstraints[actionIndex][i]});
        }

        int first = 0;
        int second = 0;
        if(!constraintsToUpdate.empty()){
            first  = constraintsToUpdate[0][0];
            second = constraintsToUpdate[0][1];
        }

        while(!constraintsToUpdate.empty()){
            float maxDelay = 0;
            for(int i : afterConstraints[second]){
                if(stn[i][1] > maxDelay){
                    maxDelay = stn[i][1];
                }
            }
            if( stn[second][0] != maxDelay){
                float newduration = stn[second][1] - stn[second][0];
                stn[second][0] = maxDelay;
                stn[second][1] = newduration;
                for(int i = 0; i < beforeConstraints[second].size(); ++i){
                    constraintsToUpdate.push_back(std::vector<int>{second, beforeConstraints[second][i]});
                }
            }

            checkedAlready[first][second] = true;

            constraintsToUpdate.erase(constraintsToUpdate.begin());
            if( !constraintsToUpdate.empty()){
                first  = constraintsToUpdate[0][0];
                second = constraintsToUpdate[0][1];
                if(checkedAlready[first][second]){
                    //std::string errorString = "Schedule invalid loop found";
                    //std::cout << errorString << std::endl;
                    return false;
                }
            }
        }
    return true;
    }

    bool Scheduler::removeOC(int first, int second){
        beforeConstraints[first].erase(std::remove(beforeConstraints[first].begin(), beforeConstraints[first].end(), second), beforeConstraints[first].end());
        afterConstraints[second].erase(std::remove(afterConstraints[second].begin(), afterConstraints[second].end(), first), afterConstraints[second].end());

        if(stn[first][1] == stn[second][0])
        {
            float maxDelay = 0;
            for(int i : afterConstraints[second]){
                if(stn[i][1] > maxDelay)
                {
                    maxDelay = stn[i][1];
                }
            }
            float duration = stn[second][1] - stn[second][0];
            stn[second][0] = maxDelay;
            stn[second][1] = duration + stn[second][0];

            std::vector<std::vector<int>> constraintsToUpdate;
            std::vector<std::vector<bool>> checkedAlready(int(stn.size()), std::vector<bool>(stn.size(), false));
            constraintsToUpdate.reserve(beforeConstraints[second].size());
            for(int i = 0; i < beforeConstraints[second].size(); ++i){
                constraintsToUpdate.push_back(std::vector<int>{second, beforeConstraints[second][i]});
            }

            if(!constraintsToUpdate.empty()){
                first  = constraintsToUpdate[0][0];
                second = constraintsToUpdate[0][1];
            }

            while(!constraintsToUpdate.empty()){

                maxDelay = 0;
                for(int i : afterConstraints[second]){
                    if(stn[i][1] > maxDelay){
                        maxDelay = stn[i][1];
                    }
                }
                if( stn[second][0] != maxDelay){
                    float newduration = stn[second][1] - stn[second][0];
                    stn[second][0] = maxDelay;
                    stn[second][1] = newduration;
                    for(int i = 0; i < beforeConstraints[second].size(); ++i){
                        constraintsToUpdate.push_back(std::vector<int>{second, beforeConstraints[second][i]});
                    }
                }

                checkedAlready[first][second] = true;

                constraintsToUpdate.erase(constraintsToUpdate.begin());
                if(!constraintsToUpdate.empty()){
                    first  = constraintsToUpdate[0][0];
                    second = constraintsToUpdate[0][1];
                    if(checkedAlready[first][second] == 1){
                        //std::string errorString = "Schedule invalid loop found";
                        //std::cout << errorString << std::endl;
                        return false;
                    }
                }
            }
        }
        return true;
    }

    bool Scheduler::removeAction(int actionID){
        while(!beforeConstraints[actionID].empty()){
            Scheduler::removeOC(actionID, beforeConstraints[actionID][0]);
        }
        for(int i=0; i < afterConstraints[actionID].size(); ++i){
            beforeConstraints[afterConstraints[actionID][i]].erase(std::remove(beforeConstraints[afterConstraints[actionID][i]].begin(), beforeConstraints[afterConstraints[actionID][i]].end(), actionID), beforeConstraints[afterConstraints[actionID][i]].end());
        }
        for(int i=0; i < disjuctiveConstraints.size(); ++i){

        }
        stn.erase(stn.begin() + actionID);
        scheduleValid = setDisjuctive();
        return true;
    }

    bool Scheduler::addAction(float duration, const std::vector<int>& orderingConstraints){
        stn.push_back(std::vector<float>{0, duration});
        for(auto & orderingConstraint : orderingConstraints){
            scheduleValid = addOC(int(stn.size()-1), orderingConstraint);
            if(!scheduleValid){
                return scheduleValid;
            }
        }
        return scheduleValid;
    }

    bool Scheduler::schedule( const std::vector<float>& durations, std::vector<std::vector<int>> orderingConstraints){
        scheduleValid = true;
        initSTN(durations);
        beforeConstraints = std::vector<std::vector<int>>(durations.size(), std::vector<int>(0));
        afterConstraints = std::vector<std::vector<int>>(durations.size(), std::vector<int>(0));
        for(auto & orderingConstraint : orderingConstraints){
            bool added = addOC(orderingConstraint[0],orderingConstraint[1]);
            if( added == 0){
                return false;
            }
        }
        scheduleValid = true;
        return true;
    }

    bool Scheduler::schedule(const std::vector<float>& durations, std::vector<std::vector<int>> orderingConstraints, std::vector<std::vector<int>> disConstraints){
        scheduleValid = false;
        initSTN(durations);

        beforeConstraints = std::vector<std::vector<int>>(durations.size(), std::vector<int>(0));
        afterConstraints = std::vector<std::vector<int>>(durations.size(), std::vector<int>(0));
        for(auto & orderingConstraint : orderingConstraints){
            bool added = addOC(orderingConstraint[0], orderingConstraint[1]);
            if(!added){
                return false;
            }
        }
        disjuctiveConstraints = disConstraints;
        disjuctiveOrderings.resize(disjuctiveConstraints.size());
        std::fill(disjuctiveOrderings.begin(), disjuctiveOrderings.end(),0);
        if(disConstraints.size() > 0){
            scheduleValid = setDisjuctive();
        }

        return scheduleValid;
    }

    //todo edit
    bool Scheduler::addAction(float duration, const std::vector<int>& orderingConstraints, std::vector<std::vector<int>> disorderingConstraints){
        stn.push_back(std::vector<float>{0, duration});
        for(auto & orderingConstraint : orderingConstraints){
            scheduleValid = addOC(int(stn.size()-1), orderingConstraint);
            if(!scheduleValid){
                return scheduleValid;
            }
        }
        disjuctiveConstraints.insert(disjuctiveConstraints.end(), disorderingConstraints.begin(), disorderingConstraints.end());
        scheduleValid = setDisjuctive();

        return scheduleValid;
    }

    float Scheduler::checkOC(int first, int second) {
        std::vector<std::vector<float>> stn_copy = stn;
        std::vector<std::vector<int>> constraintsToUpdate;
        constraintsToUpdate.push_back(std::vector<int>{first, second});

        std::vector<std::vector<bool>> checkedAlready(int(stn.size()), std::vector<bool>( stn_copy .size(), false));
        checkedAlready[first][second] = true;
        while(!constraintsToUpdate.empty()){
            if(  stn_copy [second][0] <   stn_copy [first][1]){
                float duration =  stn_copy [second][1] -  stn_copy [second][0];
                stn_copy [second][0] =  stn_copy [first][1];
                stn_copy [second][1] =  stn_copy [second][0] + duration;
                for(int i = 0; i < beforeConstraints[second].size(); ++i)
                {
                    constraintsToUpdate.push_back(std::vector<int>{second, beforeConstraints[second][i]});
                }
            }
            checkedAlready[first][second] = true;
            constraintsToUpdate.erase(constraintsToUpdate.begin());
            if( !constraintsToUpdate.empty())
            {
                first  = constraintsToUpdate[0][0];
                second = constraintsToUpdate[0][1];
                if(checkedAlready[first][second] == 1)
                {
                    //std::string errorString = "Schedule invalid loop found";
                    //std::cout << errorString << std::endl;
                    return -1;
                }
            }

        }
        float max = 0;
        for(auto & i : stn_copy){
            if(max < i[1]){
                max = i[1];
            }
        }
        return max;
    }

    void Scheduler::printSchedule(){
        std::cout << "Schedule:" << std::endl;
        for(int i=0; i < stn.size(); ++i){
            std::cout << "Action: " << i << "; Start Time= " << stn[i][0] <<  "; End Time= " << stn[i][1] << std::endl;
        }
    }

    int Scheduler::getDisjuctiveSize(){
        return disjuctiveConstraints.size();
    }

    std::string Scheduler::getDisjuctiveID(){
        return disID;
    }

    //todo edit
    Scheduler Scheduler::getShedSwitch(int i){
        int first = disjuctiveConstraints[i][0];
        int second = disjuctiveConstraints[i][1];
        int order = disjuctiveOrderings[i];
        Scheduler copySched(*this);
        if(order ==0){
            copySched.removeOC(first, second);
            copySched.addOC(second, first);
            disjuctiveOrderings[i] = 1;
            disID[i] = 1;
        }
        else{
            copySched.removeOC(second, first);
            copySched.addOC(first, second);
            disjuctiveOrderings[i] = 0;
            disID[i] = 0;
        }
        return copySched;
    }

    //todo edit
    Scheduler Scheduler::getRandomDisjunct(){
        bool complete = false;
        Scheduler copySched(*this);
        std::fill(disjuctiveOrderings.begin(),disjuctiveOrderings.end(),0);
        disID = "";
        disID.resize(disjuctiveConstraints.size(),' ');
        while(!complete)
        {
            copySched = *this;
            std::vector<int> toChange(disjuctiveConstraints.size());
            std::iota(toChange.begin(), toChange.end(), 0);
            int i = 0;
            for( i ; i < disjuctiveConstraints.size(); ++i)
            {
                int v1             = rand() % toChange.size();
                int order          = rand() % 2;
                bool allowedFirst  = copySched.checkOC(disjuctiveConstraints[i][0], disjuctiveConstraints[i][0]) >= 0;
                bool allowedSecond = copySched.checkOC(disjuctiveConstraints[i][1], disjuctiveConstraints[i][0]) >= 0;

                if(order == 0 && allowedFirst){
                    copySched.addOC(disjuctiveConstraints[i][0], disjuctiveConstraints[i][1]);
                    disjuctiveOrderings[i] = 0;
                    disID[i] = 0;
                }
                else if(allowedSecond){
                    copySched.addOC(disjuctiveConstraints[i][1], disjuctiveConstraints[i][0]);
                    disjuctiveOrderings[i] = 1;
                    disID[i] = 1;
                }
                else if(allowedFirst){
                    copySched.addOC(disjuctiveConstraints[i][0], disjuctiveConstraints[i][1]);
                    disjuctiveOrderings[i] = 0;
                    disID[i] = 0;
                }
                else{
                    break;
                }
            }
            if(i == disjuctiveConstraints.size()){
                complete = true;
                copySched.scheduleValid = true;
            }
        }
        return copySched;
    }

    //todo edit
    bool Scheduler::setDisjuctive(){
        tabu tabuSearch;
        Scheduler tabuSched =  tabuSearch.solve(10,*this);
        if(tabuSched.scheduleValid){
            *this =  tabuSched;
            return true;
        }
        else{
            return false;
        }
    }


    //todo remove action disjunctive

}



