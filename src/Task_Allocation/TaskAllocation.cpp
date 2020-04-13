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

#include "grstaps/Task_Allocation/TaskAllocation.h"
#include <boost/shared_ptr.hpp>


#include <utility>


namespace grstaps {

    TaskAllocation::TaskAllocation(vector<vector<float>>* goalDistribution, vector<vector <float>>* speciesDistribution, vector<short> startAllocation, vector<vector<float>>* noncumTraitCutoff, boost::shared_ptr<vector<int>> numSpec, vector<std::string>* actionNamesStr){
        goalTraitDistribution = goalDistribution;
        goalDistance = 0.0;
        speciesTraitDistribution = speciesDistribution;
        actionIDs = actionNamesStr;
        actionNoncumulativeTraitValue = noncumTraitCutoff;
        allocation = std::move(startAllocation);
        updateAllocationTraitDistribution();
        isGoal = checkGoalAllocation();
        scheduleType =boost::shared_ptr<int>(new int(-1));
        scheduleTime = -1;
        if(numSpec !=nullptr ){
            numSpecies = numSpec;
        }
        else{
            numSpecies = boost::shared_ptr<vector<int>>(new vector<int>);
            numSpecies->resize(speciesTraitDistribution->size(), 1);
        }
    }

    TaskAllocation::TaskAllocation(vector<vector<float>>* goalDistribution, vector<vector<float>>* speciesDistribution, vector<vector<float>>* noncumTraitCutoff, boost::shared_ptr<vector<int>> numSpec, vector<std::string>* actionNamesStr){
        goalTraitDistribution = goalDistribution;
        speciesTraitDistribution = speciesDistribution;
        actionNoncumulativeTraitValue = noncumTraitCutoff;
        goalDistance = 0.0;
        actionIDs = actionNamesStr;
        allocation.resize(goalTraitDistribution->size()*speciesTraitDistribution->size(), 0);
        scheduleType =boost::shared_ptr<int>(new int(-1));
        scheduleTime = -1;
        updateAllocationTraitDistribution();
        isGoal = checkGoalAllocation();
        if(numSpec != nullptr ){
            numSpecies = numSpec;
        }
        else{
            numSpecies = boost::shared_ptr<vector<int>>(new vector<int>);
            numSpecies->resize(speciesTraitDistribution->size(), 1);
        }
    }

    TaskAllocation::TaskAllocation()= default;

    TaskAllocation::TaskAllocation(TaskAllocation& copyAllocation){
            scheduleType = copyAllocation.scheduleType;
            speciesTraitDistribution = copyAllocation.speciesTraitDistribution;
            scheduler = copyAllocation.scheduler;
            numSpecies = copyAllocation.numSpecies;

            actionNoncumulativeTraitValue = copyAllocation.actionNoncumulativeTraitValue;
            actionIDs = copyAllocation.actionIDs;
            goalTraitDistribution = copyAllocation.goalTraitDistribution;

            scheduleTime = copyAllocation.scheduleTime;
            goalDistance = copyAllocation.goalDistance;
            isGoal = copyAllocation.isGoal;
            allocation = copyAllocation.allocation;

        }

    bool TaskAllocation::checkGoalAllocation(){
        return goalDistance <=0;
    }

    void TaskAllocation::setAllocation(vector<short> newAllocation){
        allocation = std::move(newAllocation);
        updateAllocationTraitDistribution();
    }

    void TaskAllocation::setGoalTraitDistribution(vector<vector<float>>* newGoalTraitDistribution){
        goalTraitDistribution = newGoalTraitDistribution;
        isGoal = checkGoalAllocation();
        updateAllocationTraitDistribution();
    }

    void TaskAllocation::setSpeciesTraitDistribution(vector<vector<float>>* newSpeciesTraitDistribution){
        speciesTraitDistribution = newSpeciesTraitDistribution;
        updateAllocationTraitDistribution();
    }

    void TaskAllocation::setActionNoncumulativeTraitValue(vector<vector<float>>* newActionNoncumulativeTraitValue){
        actionNoncumulativeTraitValue = newActionNoncumulativeTraitValue;
        updateAllocationTraitDistribution();
    }

    vector<vector<float>> TaskAllocation::getAllocationTraitDistribution(){
        vector<vector<float>> allocationTraitDistribution{};
        allocationTraitDistribution.resize((*goalTraitDistribution).size(),vector<float>((*speciesTraitDistribution)[0].size(),0.0));
        for (int i = 0; i < allocationTraitDistribution.size(); i++){
            for (int j = 0; j < allocationTraitDistribution[0].size(); j++){
                for(int k = 0; k < speciesTraitDistribution->size(); k++) {
                    if ((*actionNoncumulativeTraitValue)[i][j] != 0.0) {
                        if((*speciesTraitDistribution)[k][j] >= (*actionNoncumulativeTraitValue)[i][j]){
                            allocationTraitDistribution[i][j] += allocation[i*speciesTraitDistribution->size() + k];
                        }

                    }
                    else {
                        allocationTraitDistribution[i][j] += ( (*speciesTraitDistribution)[k][j] * allocation[i*speciesTraitDistribution->size() + k]);
                    }
                }
            }
        }
    }

    vector<std::string>* TaskAllocation::getActionIDs(){
        return actionIDs;
    }

    boost::shared_ptr<vector<int>> TaskAllocation::getNumSpecies(){
        return numSpecies;
    }

    vector<short> TaskAllocation::getAllocation(){
        return allocation;
    }

    vector<vector<float>>* TaskAllocation::getGoalTraitDistribution(){
        return goalTraitDistribution;
    }

    vector<vector<float>>* TaskAllocation::getSpeciesTraitDistribution(){
        return speciesTraitDistribution;
    }

    vector<vector<float>>* TaskAllocation::getActionNoncumulativeTraitValue(){
        return actionNoncumulativeTraitValue;
    }

    std::string TaskAllocation::getID(){
        std::string ID = "";
        int largestDigit = 1;
        for (int i = 0; i < numSpecies->size(); i++) {
            int digits = 0;
            int numAgents =  (*numSpecies)[i];
            do { numAgents/= 10; digits++; } while (numAgents != 0);
            if( digits > largestDigit){largestDigit = digits;}
        }

        for (int i = 0; i < allocation.size(); i++){
            std::ostringstream str;
            str << std::setw(largestDigit) << std::setfill('0') << allocation[i];
            ID.append(str.str());

        }
        return ID;
    }

    bool TaskAllocation::isGoalAllocation(){
        return this->isGoal;
    }

    float TaskAllocation::getGoalDistance(){
        return goalDistance;
    }

    boost::shared_ptr<vector<int>>  TaskAllocation::getnumSpecies(){
        return numSpecies;
    }

    bool TaskAllocation::addAgent(int agentIndex, int taskIndex){
        bool added = false;
        if(allocation[taskIndex * speciesTraitDistribution->size() + agentIndex] < (*numSpecies)[agentIndex]) {
            allocation[taskIndex *speciesTraitDistribution->size() + agentIndex] += 1;
            updateAllocationTraitDistributionAgent(agentIndex, taskIndex);
            added = true;
            isGoal = checkGoalAllocation();
        }
        return added;
    }

    void TaskAllocation::setActionIDs(vector<std::string>* newActionIDs){
        actionIDs = newActionIDs;
    }

    float TaskAllocation::getScheduleTime(int requestedScheduleType){
        if(requestedScheduleType == (*scheduleType)){
            return scheduleTime;
        }
        else{
            return 0.0;
            //scheduleType = requestedScheduleType;
            //scheduleTime = scheduler->getScheduleTime(scheduleType, allocation, (*actionIDs));
            //return scheduleTime;
        }
    }

    void TaskAllocation::setNumSpecies(boost::shared_ptr<vector<int>> newNumSpecies){
        numSpecies = newNumSpecies;
    }

    void TaskAllocation::updateAllocationTraitDistributionAgent(int agentIndex, int taskIndex){
        vector<float> allocationTraitDistribution{};
        allocationTraitDistribution.resize((*speciesTraitDistribution)[0].size(), 0.0);
        for(int k=0; k < (*speciesTraitDistribution).size(); k++) {
            if(allocation[taskIndex* speciesTraitDistribution->size() + k] != 0) {
                for (int i = 0; i < (*speciesTraitDistribution)[0].size(); i++) {
                    if( k != agentIndex){
                        allocationTraitDistribution[i] += allocation[taskIndex* speciesTraitDistribution->size() + k] * (*speciesTraitDistribution)[k][i];
                    }
                    else{
                        allocationTraitDistribution[i] += (allocation[taskIndex* speciesTraitDistribution->size() + k] - 1) * (*speciesTraitDistribution)[k][i];
                    }
                }
            }
        }
        for (int j = 0; j < allocationTraitDistribution.size(); j++){
            if ((*actionNoncumulativeTraitValue)[taskIndex][j] != 0.0){
                if((*speciesTraitDistribution)[agentIndex][j] >= (*actionNoncumulativeTraitValue)[taskIndex][j]){
                    if(allocationTraitDistribution[j] < (*goalTraitDistribution)[taskIndex][j]){
                        goalDistance -= 1;
                    }
                }
            }
            else {
                if(allocationTraitDistribution[j] < (*goalTraitDistribution)[taskIndex][j]){
                    goalDistance -=  ((*speciesTraitDistribution)[agentIndex][j]);
                }
            }
        }

        if(goalDistance < 0){
            goalDistance = 0;
        }
        isGoal = checkGoalAllocation();
    }

    void TaskAllocation::updateAllocationTraitDistribution(){
        for(auto & i : *goalTraitDistribution){
            for(int j= 0 ; j < (*goalTraitDistribution)[0].size(); j++){
                goalDistance += i[j];
            }
        }
        vector<vector<float>> allocationTraitDistribution{};
        allocationTraitDistribution.resize(goalTraitDistribution->size(),vector<float>((*goalTraitDistribution)[0].size(),0.0));
        for (int i = 0; i < allocationTraitDistribution.size(); i++){
            for (int j = 0; j < allocationTraitDistribution[0].size(); j++){
                for(int k = 0; k < speciesTraitDistribution->size(); k++) {
                    if ((*actionNoncumulativeTraitValue)[i][j] != 0.0) {
                        if((*speciesTraitDistribution)[k][j] >= (*actionNoncumulativeTraitValue)[i][j]){
                            if(allocationTraitDistribution[i][j] < (*goalTraitDistribution)[i][j]){
                                goalDistance -= allocation[i*speciesTraitDistribution->size() + k];
                            }
                            allocationTraitDistribution[i][j] += allocation[i*speciesTraitDistribution->size() + k];
                        }

                    } else {
                        if(allocationTraitDistribution[i][j] < (*goalTraitDistribution)[i][j]){
                            goalDistance -=  ( (*speciesTraitDistribution)[k][j] * allocation[i*speciesTraitDistribution->size() + k]);
                        }
                        allocationTraitDistribution[i][j] += ( (*speciesTraitDistribution)[k][j] * allocation[i*speciesTraitDistribution->size() + k]);
                    }
                }
            }
        }
        if(goalDistance < 0){
            goalDistance = 0;
        }
        isGoal = checkGoalAllocation();
    }

    void TaskAllocation::checkSize(){
        std::cout << " 1) vector<vector<float>>* goalTraitDistribution{}= " << sizeof(goalTraitDistribution) << std::endl;
        std::cout << " 2) vector<vector<float>>* speciesTraitDistribution{};= " << sizeof(speciesTraitDistribution) << std::endl;
        std::cout << " 3) vector<vector<float>>* actionNoncumulativeTraitValue{};= " << sizeof(actionNoncumulativeTraitValue) << std::endl;
        std::cout << " 4) Scheduler* scheduler{};= " << sizeof(scheduler) << std::endl;
        std::cout << " 5) const vector<std::string>* actionIDs;= " << sizeof(actionIDs) << std::endl;
        std::cout << " 6) vector<int>* numSpecies;" << sizeof(numSpecies) << std::endl;


        std::cout << " 7) shared pointer scheduleType{};= " << sizeof(scheduleType) << std::endl;
        std::cout << " 8) float scheduleTime{};= " << sizeof(scheduleTime) << std::endl;
        std::cout << " 9) float goalDistance{};= " << sizeof(goalDistance) << std::endl;
        std::cout << "10) bool isGoal{};= " << sizeof(isGoal) << std::endl;


        std::cout << "11) vector<short> allocation;= " << (sizeof(std::vector<short>) +(sizeof(short) * allocation.size())) << std::endl;

        int total = sizeof(goalTraitDistribution) + sizeof(speciesTraitDistribution) + sizeof(actionNoncumulativeTraitValue) + sizeof(scheduler) + sizeof(actionIDs) + sizeof(numSpecies) + sizeof(scheduleType) + sizeof(scheduleTime) + sizeof(goalDistance) + sizeof(isGoal) +(sizeof(std::vector<short>) +(sizeof(short) * allocation.size()));
        std::cout << "Total Memory Usage= "<< total << std::endl;
    }

    void TaskAllocation::addAction(const vector<float>& actionRequirements, const vector<float>& noncumTraitCutoff, const std::string newActionID){
        actionIDs->push_back(newActionID);
        goalTraitDistribution->push_back(actionRequirements);
        actionNoncumulativeTraitValue->push_back(noncumTraitCutoff);

        vector<int> emptyVect(speciesTraitDistribution->size(), 0.0);
        allocation.insert(allocation.end(), emptyVect.begin(), emptyVect.end());
        if(isGoal) {
            isGoal = checkGoalAllocation();
        }
        for(int i=0; i < actionRequirements.size(); i++){
            goalDistance += actionRequirements[i];
        }
    }

    void TaskAllocation::addAction(vector<vector<float>>* actionRequirements, vector<vector<float>>* nonCumActionRequirements, float goalDistAdd, vector<std::string>* newActionID){
        goalTraitDistribution = actionRequirements;
        actionNoncumulativeTraitValue = nonCumActionRequirements;
        actionIDs = newActionID;

        vector<int> emptyVect(speciesTraitDistribution->size(), 0.0);
        allocation.insert(allocation.end(), emptyVect.begin(), emptyVect.end());


        goalDistance += goalDistAdd;
        if(isGoal) {
            isGoal = checkGoalAllocation();
        }
    }

} //namespace grstaps