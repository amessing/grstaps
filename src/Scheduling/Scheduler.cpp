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
    tabu tabuSearch;

    Scheduler::Scheduler(){
        scheduleValid = true;
        makeSpan = -1;
        constraintsToUpdate.reserve(1000);
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
        copySTN = toCopy.stn;
    }

    float Scheduler::initSTN(const std::vector<float>& durations){
        stn.clear();
        for(float duration : durations){
            stn.emplace_back(std::vector<float>{0, duration});
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

    float Scheduler::getMakeSpanSTN(std::vector<std::vector<float>>& stnCopy){
            float max = 0;
            for(auto & i : stnCopy){
                if(max < i[1]){
                    max = i[1];
                }
            }
            return max;
    }

    bool Scheduler::checkConcurrent(int first, int second) {
        return (stn[first][1] < stn[second][0] || stn[first][0] > stn[second][1]);
    }

    bool Scheduler::addOC(int first, int second) {
        int originalFirst = first;
        constraintsToUpdate.clear();
        beforeConstraints[first].emplace_back(second);
        afterConstraints[second].emplace_back(first);
        constraintsToUpdate.emplace_back(first);
        constraintsToUpdate.emplace_back(second);
        int constrainToAdjust = 0;
        while(constrainToAdjust < constraintsToUpdate.size()){
            if( stn[second][0] <  stn[first][1]){
                float duration = stn[second][1] - stn[second][0];
                stn[second][0] = stn[first][1];
                stn[second][1] = stn[second][0] + duration;
                for(int i = 0; i < beforeConstraints[second].size(); ++i)
                {
                    constraintsToUpdate.emplace_back(second);
                    constraintsToUpdate.emplace_back(beforeConstraints[second][i]);
                }
            }
            constrainToAdjust += 2;
            if(constrainToAdjust < constraintsToUpdate.size())
            {
                first  = constraintsToUpdate[constrainToAdjust];
                second = constraintsToUpdate[constrainToAdjust + 1];
                if(second == originalFirst){
                    scheduleValid = false;
                    return scheduleValid;
                }
            }

        }

        return scheduleValid;
    }

    float Scheduler::addOCTemp(int first, int second, std::vector<std::vector<float>> stnCopy, std::vector<std::vector<int>>& beforeConstraintVec, std::vector<std::vector<int>>& afterConstraintVec) {
            constraintsToUpdate.clear();
            int originalFirst = first;
            int originalSecond = second;
            constraintsToUpdate.push_back(first);
            constraintsToUpdate.push_back(second);
            int constrainToAdjust = 0;
            float makeSpanNew = makeSpan;
            while(constrainToAdjust < constraintsToUpdate.size())
            {
                if(stnCopy[second][0] < stnCopy[first][1])
                {
                    float duration     = stnCopy[second][1] - stnCopy[second][0];
                    stnCopy[second][0] = stnCopy[first][1];
                    stnCopy[second][1] = stnCopy[second][0] + duration;
                    if( stnCopy[second][1] > makeSpanNew){
                        makeSpanNew = stnCopy[second][1];
                    }
                    for(int i = 0; i < beforeConstraints[second].size(); ++i)
                    {
                        if(second != originalSecond || beforeConstraints[second][i] != originalFirst)
                        {
                            constraintsToUpdate.emplace_back(second);
                            constraintsToUpdate.emplace_back(beforeConstraints[second][i]);
                        }
                    }
                }
                // constraintsToUpdate.erase(constraintsToUpdate.begin());
                constrainToAdjust += 2;
                if(constrainToAdjust < constraintsToUpdate.size())
                {
                    first  = constraintsToUpdate[constrainToAdjust];
                    second = constraintsToUpdate[constrainToAdjust + 1];
                    if(second == originalFirst)
                    {
                        return -1;
                    }
                }
            }
            return makeSpanNew;
        }

    bool Scheduler::addOCTime(int first, int second, std::vector<std::vector<float>>& stnCopy, std::vector<std::vector<int>>& beforeConstraintVec, std::vector<std::vector<int>>& afterConstraintVec) {
        constraintsToUpdate.clear();
        int originalFirst = first;
        int originalSecond = second;
        constraintsToUpdate.push_back(first);
        constraintsToUpdate.push_back(second);
        int constrainToAdjust = 0;
        while(constrainToAdjust < constraintsToUpdate.size())
        {
            if(stnCopy[second][0] < stnCopy[first][1])
            {
                float duration     = stnCopy[second][1] - stnCopy[second][0];
                stnCopy[second][0] = stnCopy[first][1];
                stnCopy[second][1] = stnCopy[second][0] + duration;
                for(int i = 0; i < beforeConstraints[second].size(); ++i)
                {
                    if(second != originalSecond || beforeConstraints[second][i] != originalFirst)
                    {
                        constraintsToUpdate.emplace_back(second);
                        constraintsToUpdate.emplace_back(beforeConstraints[second][i]);
                    }
                }
            }
            // constraintsToUpdate.erase(constraintsToUpdate.begin());
            constrainToAdjust += 2;
            if(constrainToAdjust < constraintsToUpdate.size())
            {
                first  = constraintsToUpdate[constrainToAdjust];
                second = constraintsToUpdate[constrainToAdjust + 1];
                if(second == originalFirst)
                {
                    return false;
                }
            }
        }
        makeSpan = -1;
        return true;
    }

    void Scheduler::removeOC(int first, int second){
            constraintsToUpdate.clear();
            beforeConstraints[first].erase(
                std::remove(beforeConstraints[first].begin(), beforeConstraints[first].end(), second),
                beforeConstraints[first].end());
            afterConstraints[second].erase(
                std::remove(afterConstraints[second].begin(), afterConstraints[second].end(), first),
                afterConstraints[second].end());

            if(stn[first][1] == stn[second][0])
            {
                float maxDelay = 0;
                for(int i: afterConstraints[second])
                {
                    if(stn[i][1] > maxDelay)
                    {
                        maxDelay = stn[i][1];
                    }
                }
                float duration = stn[second][1] - stn[second][0];
                stn[second][0] = maxDelay;
                stn[second][1] = duration + stn[second][0];

                //constraintsToUpdate.reserve(beforeConstraints[second].size());
                for(int i = 0; i < beforeConstraints[second].size(); ++i)
                {
                    constraintsToUpdate.emplace_back(second);
                    constraintsToUpdate.emplace_back(beforeConstraints[second][i]);
                }

                if(!constraintsToUpdate.empty())
                {
                    first  = constraintsToUpdate[0];
                    second = constraintsToUpdate[1];
                }
                int constrainToAdjust = 0;
                while(constrainToAdjust < constraintsToUpdate.size())
                {
                    maxDelay = 0;
                    for(int i: afterConstraints[second])
                    {
                        if(stn[i][1] > maxDelay)
                        {
                            maxDelay = stn[i][1];
                        }
                    }
                    if(stn[second][0] != maxDelay)
                    {
                        float newduration = stn[second][1] - stn[second][0];
                        stn[second][0]    = maxDelay;
                        stn[second][1]    = stn[second][0] + newduration;
                        for(int i = 0; i < beforeConstraints[second].size(); ++i)
                        {
                            constraintsToUpdate.emplace_back(second);
                            constraintsToUpdate.emplace_back(beforeConstraints[second][i]);
                        }

                    }
                    // constraintsToUpdate.erase(constraintsToUpdate.begin());
                    constrainToAdjust += 2;
                    if(constrainToAdjust < constraintsToUpdate.size())
                    {
                        first  = constraintsToUpdate[constrainToAdjust];
                        second = constraintsToUpdate[constrainToAdjust + 1];
                    }
                }
            }
            makeSpan = -1;
    }

    void Scheduler::removeOCTime(int first, int second, std::vector<std::vector<float>>& stnCopy){
        constraintsToUpdate.clear();
        if(stnCopy[first][1] == stnCopy[second][0])
        {
            float maxDelay = 0;
            for(int i: afterConstraints[second])
            {
                if(i != first && stnCopy[i][1] > maxDelay)
                {
                    maxDelay = stnCopy[i][1];
                }
            }
            float duration = stnCopy[second][1] - stnCopy[second][0];
            stnCopy[second][0] = maxDelay;
            stnCopy[second][1] = duration + stnCopy[second][0];

            for(int i = 0; i < beforeConstraints[second].size(); ++i)
            {
                constraintsToUpdate.emplace_back(second);
                constraintsToUpdate.emplace_back(beforeConstraints[second][i]);
            }
            if(!constraintsToUpdate.empty())
            {
                first  = constraintsToUpdate[0];
                second = constraintsToUpdate[1];
            }
            int constrainToAdjust = 0;
            while(constrainToAdjust < constraintsToUpdate.size())
            {
                maxDelay = 0;
                for(int i: afterConstraints[second])
                {
                    if(stnCopy[i][1] > maxDelay)
                    {
                        maxDelay = stnCopy[i][1];
                    }
                }
                if(stnCopy[second][0] != maxDelay)
                {
                    float newduration = stnCopy[second][1] - stnCopy[second][0];
                    stnCopy[second][0]    = maxDelay;
                    stnCopy[second][1]    = stnCopy[second][0] +newduration;
                    for(int i = 0; i < beforeConstraints[second].size(); ++i)
                    {
                        constraintsToUpdate.emplace_back(second);
                        constraintsToUpdate.emplace_back(beforeConstraints[second][i]);
                    }
                }
                // constraintsToUpdate.erase(constraintsToUpdate.begin());
                constrainToAdjust += 2;
                if(constrainToAdjust < constraintsToUpdate.size())
                {
                    first  = constraintsToUpdate[constrainToAdjust];
                    second = constraintsToUpdate[constrainToAdjust + 1];
                }
            }
        }
    }

    bool Scheduler::increaseActionTime(int actionIndex, float duration){
        stn[actionIndex][1] = stn[actionIndex][0] + duration;
        std::vector<std::vector<int>> constraintsToUpdate;
        std::vector<std::vector<bool>> checkedAlready(int(stn.size()), std::vector<bool>(stn.size(), false));
        int second = actionIndex;
        int first = actionIndex;

        constraintsToUpdate.reserve(beforeConstraints[second].size());
        for(int i = 0; i < beforeConstraints[second].size(); ++i){
            constraintsToUpdate.emplace_back(std::vector<int>{second, beforeConstraints[second][i]});
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
                    constraintsToUpdate.emplace_back(std::vector<int>{second, beforeConstraints[second][i]});
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
            constraintsToUpdate.emplace_back(std::vector<int>{actionIndex, beforeConstraints[actionIndex][i]});
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
                    constraintsToUpdate.emplace_back(std::vector<int>{second, beforeConstraints[second][i]});
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
        setDisjuctive();
        return true;
    }

    bool Scheduler::addAction(float duration, const std::vector<int>& orderingConstraints){
        stn.emplace_back(std::vector<float>{0, duration});
        for(auto & orderingConstraint : orderingConstraints){
            addOC(int(stn.size()-1), orderingConstraint);
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
        scheduleValid = true;
        initSTN(durations);
        makeSpan = -1;
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
            setDisjuctive();
        }

        return scheduleValid;
    }

    bool Scheduler::addAction(float duration, const std::vector<int>& orderingConstraints, std::vector<std::vector<int>> disorderingConstraints){
        stn.emplace_back(std::vector<float>{0, duration});
        for(auto & orderingConstraint : orderingConstraints){
            addOC(int(stn.size()-1), orderingConstraint);
            if(!scheduleValid){
                return scheduleValid;
            }
        }
        disjuctiveConstraints.insert(disjuctiveConstraints.end(), disorderingConstraints.begin(), disorderingConstraints.end());
        setDisjuctive();

        return scheduleValid;
    }

    float Scheduler::checkOC(int first, int second) {
        std::vector<std::vector<int>> constraintsToUpdates;
        auto stnCopy = stn;
        int originalFirst = first;
        int originalSecond = second;
        constraintsToUpdates.push_back({first, second});
        robin_hood::unordered_map<double, int> checkedAlready;
        checkedAlready[double(first + 1/(double(second)+2))] = 1;
        int constrainToAdjust = 0;
        while(constrainToAdjust < constraintsToUpdates.size()){
            if( stnCopy[second][0] <  stnCopy[first][1]){
                float duration = stnCopy[second][1] - stnCopy[second][0];
                stnCopy[second][0] = stnCopy[first][1];
                stnCopy[second][1] = stnCopy[second][0] + duration;
                for(int i = 0; i < beforeConstraints[second].size(); ++i){
                    constraintsToUpdates.push_back({second, beforeConstraints[second][i]});
                }
                if(second == originalFirst){
                    constraintsToUpdates.push_back({originalFirst, originalSecond});
                }
            }
            checkedAlready[double(first + 1/(double(second)+2))] = 1;
            //constraintsToUpdate.erase(constraintsToUpdate.begin());
            constrainToAdjust += 1;
            if(constrainToAdjust != constraintsToUpdates.size())
            {
                first  = constraintsToUpdates[constrainToAdjust][0];
                second = constraintsToUpdates[constrainToAdjust][1];
                if(checkedAlready.find(double(first + 1/(double(second)+2))) != checkedAlready.end()){
                    return false;
                }
            }

        }
        constraintsToUpdates.clear();
        makeSpan = -1;
        return true;

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

    bool Scheduler::getShedSwitch(int i){
        bool valid;
        if(disjuctiveOrderings[i] ==0){
            this->removeOC(disjuctiveConstraints[i][0], disjuctiveConstraints[i][1]);
            valid = this->addOC(disjuctiveConstraints[i][1], disjuctiveConstraints[i][0]);
            if(valid){
                this->disjuctiveOrderings[i] = 1;
                this->disID[i]               = 1;
            }
            else{
                this->removeOC(disjuctiveConstraints[i][1], disjuctiveConstraints[i][0]);
                this->addOC(disjuctiveConstraints[i][0], disjuctiveConstraints[i][1]);
            }
        }
        else{
            this->removeOC(disjuctiveConstraints[i][1], disjuctiveConstraints[i][0]);
            valid = this->addOC(disjuctiveConstraints[i][0],disjuctiveConstraints[i][1]);
            if(valid){
                this->disjuctiveOrderings[i] = 0;
                this->disID[i]               = 0;
            }
            else{
                this->removeOC(disjuctiveConstraints[i][0], disjuctiveConstraints[i][1]);
                this->addOC(disjuctiveConstraints[i][1], disjuctiveConstraints[i][0]);
            }
        }
        return valid;
    }

    double Scheduler::getShedSwitchTime(int disIndex){
        bool valid;
        copySTN = stn;
        if(disjuctiveOrderings[disIndex] ==0){
            //copySched.removeOC(disjuctiveConstraints[disIndex][0], disjuctiveConstraints[disIndex][1]);
            removeOCTime(disjuctiveConstraints[disIndex][0], disjuctiveConstraints[disIndex][1], copySTN);
            //valid = copySched.addOC(disjuctiveConstraints[disIndex][1],disjuctiveConstraints[disIndex][0]);
            valid = addOCTime(disjuctiveConstraints[disIndex][1], disjuctiveConstraints[disIndex][0],copySTN, beforeConstraints, afterConstraints);
            if(valid){
                return getMakeSpanSTN(copySTN);
            }
            else{
                return  -1;
            }
        }
        else{
            //copySched.removeOC(disjuctiveConstraints[disIndex][1], disjuctiveConstraints[disIndex][0]);
            removeOCTime(disjuctiveConstraints[disIndex][1], disjuctiveConstraints[disIndex][0], copySTN);
            //valid = copySched.addOC(disjuctiveConstraints[disIndex][0],disjuctiveConstraints[disIndex][1]);
            valid = addOCTime(disjuctiveConstraints[disIndex][0],disjuctiveConstraints[disIndex][1], copySTN, beforeConstraints, afterConstraints);
            if(valid){
                return getMakeSpanSTN(copySTN);
            }
            else{
                return  -1;
            }
        }

    }

    //todo edit
    void Scheduler::getRandomDisjunct(Scheduler& copySched){
        bool complete = false;
        copySched = (*this);
        std::fill(disjuctiveOrderings.begin(),disjuctiveOrderings.end(),0);
        disID = std::string(disjuctiveConstraints.size(),' ');
        while(!complete)
        {
            copySched = *this;
            int i = 0;
            for( i ; i < disjuctiveConstraints.size(); ++i)
            {
                if(copySched.stn[disjuctiveConstraints[i][0]][0] > copySched.stn[disjuctiveConstraints[i][1]][1]){
                    copySched.addOC(disjuctiveConstraints[i][1], disjuctiveConstraints[i][0]);
                    disjuctiveOrderings[i] = 0;
                    disID[i] = 0;
                }
                else if(copySched.stn[disjuctiveConstraints[i][0]][1] < copySched.stn[disjuctiveConstraints[i][1]][0]){
                    copySched.addOC(disjuctiveConstraints[i][0], disjuctiveConstraints[i][1]);
                    disjuctiveOrderings[i] = 1;
                    disID[i] = 1;
                }
                else
                {
                    int order = rand() % 2;
                    bool allowedFirst =
                        copySched.checkOC(disjuctiveConstraints[i][0], disjuctiveConstraints[i][1]) >= 0;
                    bool allowedSecond =
                        copySched.checkOC(disjuctiveConstraints[i][1], disjuctiveConstraints[i][0]) >= 0;

                    if( allowedFirst && (order == 0 || (!allowedSecond)))
                    {
                        copySched.addOC(disjuctiveConstraints[i][0], disjuctiveConstraints[i][1]);
                        disjuctiveOrderings[i] = 0;
                        disID[i]               = 0;
                    }
                    else if(allowedSecond)
                    {
                        copySched.addOC(disjuctiveConstraints[i][1], disjuctiveConstraints[i][0]);
                        disjuctiveOrderings[i] = 1;
                        disID[i]               = 1;
                    }
                    else
                    {
                        i = 0;
                        break;
                    }
                }
            }
            if(i == disjuctiveConstraints.size()){
                complete = true;
                copySched.scheduleValid = true;
            }
        }
        //return copySched;
    }

    bool Scheduler::setDisjuctive(){
        Scheduler tabuSched =  tabuSearch.solve(1,*this);
        if(tabuSched.scheduleValid){
            *this =  tabuSched;
            return true;
        }
        else{
            return false;
        }
    }

}



