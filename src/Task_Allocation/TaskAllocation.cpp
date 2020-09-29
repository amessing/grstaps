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
#include <utility>

#include <boost/shared_ptr.hpp>
#include <grstaps/Connections/taskAllocationToScheduling.h>

float epsilon = 0.0001;

namespace grstaps
{
    TaskAllocation::TaskAllocation(bool useSpec,
                                   const boost::shared_ptr<vector<vector<float>>> goalDistribution,
                                   vector<vector<float>>* speciesDistribution,
                                   vector<short> startAllocation,
                                   boost::shared_ptr<vector<vector<float>>> noncumTraitCutoff,
                                   taskAllocationToScheduling taToSched,
                                   boost::shared_ptr<vector<float>> actionDur,
                                   boost::shared_ptr<vector<vector<int>>> orderingCon,
                                   const boost::shared_ptr<vector<int>> numSpec,
                                   int speedInd,
                                   int mpIndex)
    {
        action_dynamics               = vector<int>(goalDistribution->size(),-1);
        mp_Index                      = mpIndex;
        speedIndex                    = speedInd;
        usingSpecies                  = useSpec;
        taToScheduling                = taToSched;
        goalTraitDistribution         = goalDistribution;
        goalDistance                  = 0.0;
        speciesTraitDistribution      = speciesDistribution;
        actionDurations               = std::move(actionDur);
        orderingConstraints           = std::move(orderingCon);
        actionNoncumulativeTraitValue = std::move(noncumTraitCutoff);
        allocation                    = std::move(startAllocation);
        allocationTraitDistribution =
            vector<vector<float>>(goalTraitDistribution->size(), std::vector<float>((*goalDistribution)[0].size(), 0));
        updateAllocationTraitDistribution();
        isGoal                = checkGoalAllocation();
        requirementsRemaining = *goalDistribution;
        scheduleTime          = -1;
        if(numSpec != nullptr)
        {
            numSpecies = numSpec;
        }
        else
        {
            numSpecies = boost::shared_ptr<vector<int>>(new vector<int>);
            numSpecies->resize(speciesTraitDistribution->size(), 1);
        }

        traitTeamMax = boost::shared_ptr<vector<float>>(new vector<float>(speciesDistribution[0].size(), 0));
        maxSpeed = 0;
        for(int i = 0; i < numSpec->size(); ++i)
        {
            for(int j = 0; j < (*speciesDistribution)[0].size(); ++j)
            {
                (*traitTeamMax)[j] += (*speciesDistribution)[i][j] * (*numSpec)[i];
                if(j == speedInd && maxSpeed < (*speciesDistribution)[i][j]){
                    maxSpeed = (*speciesDistribution)[i][j];
                }
            }
        }
        float startGoalDistance = goalDistance;
        startingGoalDistance    = &startGoalDistance;
    }

    TaskAllocation::TaskAllocation(bool useSpec,
                                   const boost::shared_ptr<vector<vector<float>>> goalDistribution,
                                   vector<vector<float>>* speciesDistribution,
                                   boost::shared_ptr<vector<vector<float>>> noncumTraitCutoff,
                                   taskAllocationToScheduling taToSched,
                                   boost::shared_ptr<vector<float>> actionDur,
                                   boost::shared_ptr<vector<vector<int>>> orderingCon,
                                   boost::shared_ptr<vector<int>> numSpec,
                                   int speedInd,
                                   int mpInd)
    {
        action_dynamics               = vector<int>(goalDistribution->size(),-1);
        mp_Index                      = mpInd;
        usingSpecies                  = useSpec;
        taToScheduling                = taToSched;
        actionDurations               = std::move(actionDur);
        orderingConstraints           = std::move(orderingCon);
        goalTraitDistribution         = goalDistribution;
        speciesTraitDistribution      = speciesDistribution;
        actionNoncumulativeTraitValue = std::move(noncumTraitCutoff);
        requirementsRemaining         = *goalDistribution;
        goalDistance                  = 0.0;
        speedIndex                    = speedInd;
        allocation.resize(goalTraitDistribution->size() * speciesTraitDistribution->size(), 0);
        scheduleTime = -1;
        allocationTraitDistribution =
            vector<vector<float>>(goalTraitDistribution->size(), std::vector<float>((*goalDistribution)[0].size(), 0));
        updateAllocationTraitDistribution();
        isGoal = checkGoalAllocation();
        if(numSpec != nullptr)
        {
            numSpecies = numSpec;
        }
        else
        {
            numSpecies = boost::shared_ptr<vector<int>>(new vector<int>);
            numSpecies->resize(speciesTraitDistribution->size(), 1);
        }

        traitTeamMax = boost::shared_ptr<vector<float>>(new vector<float>((*speciesDistribution)[0].size(), 0));
        maxSpeed = 0;
        for(int i = 0; i < numSpec->size(); ++i)
        {
            for(int j = 0; j < (*speciesDistribution)[0].size(); ++j)
            {
                (*traitTeamMax)[j] += (*speciesDistribution)[i][j] * (*numSpec)[i];
                if(j == speedInd && maxSpeed < (*speciesDistribution)[i][j]){
                    maxSpeed = (*speciesDistribution)[i][j];
                }
            }
        }
        float startGoalDistance = goalDistance;
        startingGoalDistance    = &startGoalDistance;
    }

    TaskAllocation::TaskAllocation(const TaskAllocation& copyAllocation)
    {
        usingSpecies             = copyAllocation.usingSpecies;
        speciesTraitDistribution = copyAllocation.speciesTraitDistribution;
        taToScheduling           = copyAllocation.taToScheduling;
        numSpecies               = copyAllocation.numSpecies;

        actionNoncumulativeTraitValue = copyAllocation.actionNoncumulativeTraitValue;
        actionDurations               = copyAllocation.actionDurations;
        orderingConstraints           = copyAllocation.orderingConstraints;
        goalTraitDistribution         = copyAllocation.goalTraitDistribution;

        scheduleTime                = copyAllocation.scheduleTime;
        goalDistance                = copyAllocation.goalDistance;
        isGoal                      = copyAllocation.isGoal;
        allocation                  = copyAllocation.allocation;
        allocationTraitDistribution = copyAllocation.allocationTraitDistribution;
        traitTeamMax                = copyAllocation.traitTeamMax;
        requirementsRemaining       = copyAllocation.requirementsRemaining;
        startingGoalDistance        = copyAllocation.startingGoalDistance;
        speedIndex                  = copyAllocation.speedIndex;
        maxSpeed                    = copyAllocation.maxSpeed;
        mp_Index                    = copyAllocation.mp_Index;
        action_dynamics             = copyAllocation.action_dynamics;
    }

    bool TaskAllocation::checkGoalAllocation() const
    {
        return goalDistance <= epsilon;
    }

    [[maybe_unused]] void TaskAllocation::setAllocation(const std::vector<short>& newAllocation)
    {
        allocation = newAllocation;
        updateAllocationTraitDistribution();
    }

    [[maybe_unused]] void TaskAllocation::setGoalTraitDistribution(
        boost::shared_ptr<vector<vector<float>>> newGoalTraitDistribution)
    {
        goalTraitDistribution = std::move(newGoalTraitDistribution);
        isGoal                = checkGoalAllocation();
        updateAllocationTraitDistribution();
    }

    [[maybe_unused]] void TaskAllocation::setSpeciesTraitDistribution(
        vector<vector<float>>* newSpeciesTraitDistribution)
    {
        speciesTraitDistribution = newSpeciesTraitDistribution;
        updateAllocationTraitDistribution();
    }

    [[maybe_unused]] void TaskAllocation::setActionNoncumulativeTraitValue(
        boost::shared_ptr<vector<vector<float>>> newActionNoncumulativeTraitValue)
    {
        actionNoncumulativeTraitValue = std::move(newActionNoncumulativeTraitValue);
        updateAllocationTraitDistribution();
    }

    [[maybe_unused]] const vector<vector<float>>& TaskAllocation::getAllocationTraitDistribution() const
    {
        return allocationTraitDistribution;
    }

    boost::shared_ptr<vector<int>> TaskAllocation::getNumSpecies()
    {
        return numSpecies;
    }

    const std::vector<short>& TaskAllocation::getAllocation() const
    {
        return allocation;
    }

    boost::shared_ptr<vector<vector<float>>> TaskAllocation::getGoalTraitDistribution() const
    {
        return goalTraitDistribution;
    }

    vector<vector<float>>* TaskAllocation::getSpeciesTraitDistribution()
    {
        return speciesTraitDistribution;
    }

    boost::shared_ptr<vector<vector<float>>> TaskAllocation::getActionNoncumulativeTraitValue()
    {
        return actionNoncumulativeTraitValue;
    }

    std::string TaskAllocation::getID()
    {
        std::string ID;
        int largestDigit = 1;
        for(int i = 0; i < numSpecies->size(); i++)
        {
            int digits    = 0;
            int numAgents = (*numSpecies)[i];
            do
            {
                numAgents /= 10;
                digits++;
            } while(numAgents != 0);
            if(digits > largestDigit)
            {
                largestDigit = digits;
            }
        }

        for(short i: allocation)
        {
            std::ostringstream str;
            str << std::setw(largestDigit) << std::setfill('0') << i;
            ID.append(str.str());
        }
        return ID;
    }

    bool TaskAllocation::isGoalAllocation() const
    {
        return this->isGoal;
    }

    float TaskAllocation::getGoalDistance() const
    {
        return goalDistance;
    }

    [[maybe_unused]] boost::shared_ptr<vector<int>> TaskAllocation::getnumSpecies()
    {
        return numSpecies;
    }

    bool TaskAllocation::addAgent(int agentIndex, int taskIndex)
    {
        bool added = false;
        if(allocation[taskIndex * speciesTraitDistribution->size() + agentIndex] < (*numSpecies)[agentIndex])
        {
            allocation[taskIndex * speciesTraitDistribution->size() + agentIndex] += 1;
            updateAllocationTraitDistributionAgent(agentIndex, taskIndex);
            added  = true;
            isGoal = checkGoalAllocation();
            action_dynamics[taskIndex] = (*speciesTraitDistribution)[agentIndex][mp_Index];
        }
        scheduleTime = -1;
        return added;
    }

    void TaskAllocation::setNumSpecies(boost::shared_ptr<vector<int>> newNumSpecies)
    {
        numSpecies = std::move(newNumSpecies);
    }

    void TaskAllocation::checkSize()
    {
        std::cout << " 1) vector<vector<float>>* goalTraitDistribution{}= " << sizeof(goalTraitDistribution)
                  << std::endl;
        std::cout << " 2) vector<vector<float>>* speciesTraitDistribution{};= " << sizeof(speciesTraitDistribution)
                  << std::endl;
        std::cout << " 3) vector<vector<float>>* actionNoncumulativeTraitValue{};= "
                  << sizeof(actionNoncumulativeTraitValue) << std::endl;
        std::cout << " 4) Scheduler* taToSchedule{};= " << sizeof(taToScheduling) << std::endl;
        std::cout << " 5) const vector<float>* actionDurations;= " << sizeof(actionDurations) << std::endl;
        std::cout << " 6) vector<int>* numSpecies;" << sizeof(numSpecies) << std::endl;
        std::cout << " 5) const vector<vector<float>* orderingConstraints;= " << sizeof(actionDurations) << std::endl;
        std::cout << " 8) float scheduleTime{};= " << sizeof(scheduleTime) << std::endl;
        std::cout << " 9) float goalDistance{};= " << sizeof(goalDistance) << std::endl;
        std::cout << "10) bool isGoal{};= " << sizeof(isGoal) << std::endl;
        std::cout << "11) vector<short> allocation;= "
                  << (sizeof(std::vector<short>) + (sizeof(short) * allocation.size())) << std::endl;

        int total = sizeof(goalTraitDistribution) + sizeof(speciesTraitDistribution) +
                    sizeof(actionNoncumulativeTraitValue) + sizeof(taToScheduling) + 2 * sizeof(actionDurations) +
                    sizeof(numSpecies) + sizeof(scheduleTime) + sizeof(goalDistance) + sizeof(isGoal) +
                    (sizeof(std::vector<short>) + (sizeof(short) * allocation.size()));
        std::cout << "Total Memory Usage= " << total << std::endl;
    }

    // todo update
    void TaskAllocation::updateAllocationTraitDistributionAgent(int agentIndex, int taskIndex)
    {
        for(int i = 0; i < (*speciesTraitDistribution)[0].size(); i++)
        {
            float allocBefore = allocationTraitDistribution[taskIndex][i];
            allocationTraitDistribution[taskIndex][i] += ((*speciesTraitDistribution)[agentIndex][i]);
            if((*actionNoncumulativeTraitValue)[taskIndex][i] != 0.0)
            {
                if((*speciesTraitDistribution)[agentIndex][i] >= (*actionNoncumulativeTraitValue)[taskIndex][i])
                {
                    if(allocBefore < (*goalTraitDistribution)[taskIndex][i])
                    {
                        goalDistance -= 1;
                        requirementsRemaining[taskIndex][i] -= 1;
                    }
                }
            }
            else
            {
                if(allocBefore < (*goalTraitDistribution)[taskIndex][i])
                {
                    float allocAfter = allocBefore + ((*speciesTraitDistribution)[agentIndex][i]);
                    if(allocAfter < (*goalTraitDistribution)[taskIndex][i])
                    {
                        goalDistance -= ((*speciesTraitDistribution)[agentIndex][i]);
                        requirementsRemaining[taskIndex][i] -= ((*speciesTraitDistribution)[agentIndex][i]);
                    }
                    else
                    {
                        goalDistance -= (*goalTraitDistribution)[taskIndex][i] - allocBefore;
                        requirementsRemaining[taskIndex][i] -= (*goalTraitDistribution)[taskIndex][i] - allocBefore;
                    }
                }
            }
        }

        if(goalDistance <= epsilon)
        {
            goalDistance = 0;
        }
        isGoal       = checkGoalAllocation();
        scheduleTime = -1;
    }

    // todo update
    void TaskAllocation::updateAllocationTraitDistribution()
    {
        allocationTraitDistribution.resize(goalTraitDistribution->size(),
                                           vector<float>((*goalTraitDistribution)[0].size(), 0.0));
        for(int i = 0; i < allocationTraitDistribution.size(); i++)
        {
            for(int j = 0; j < allocationTraitDistribution[0].size(); j++)
            {
                for(int k = 0; k < speciesTraitDistribution->size(); k++)
                {
                    if((*actionNoncumulativeTraitValue)[i][j] != 0.0)
                    {
                        if((*speciesTraitDistribution)[k][j] >= (*actionNoncumulativeTraitValue)[i][j])
                        {
                            allocationTraitDistribution[i][j] += allocation[i * speciesTraitDistribution->size() + k];
                        }
                        if(allocationTraitDistribution[i][j] > (*goalTraitDistribution)[i][j])
                        {
                            allocationTraitDistribution[i][j] = (*goalTraitDistribution)[i][j];
                        }
                    }
                    else
                    {
                        allocationTraitDistribution[i][j] +=
                            ((*speciesTraitDistribution)[k][j] * allocation[i * speciesTraitDistribution->size() + k]);
                        if(allocationTraitDistribution[i][j] > (*goalTraitDistribution)[i][j])
                        {
                            allocationTraitDistribution[i][j] = (*goalTraitDistribution)[i][j];
                        }
                    }
                }
            }
        }
        for(int i = 0; i < (*goalTraitDistribution).size(); i++)
        {
            for(int j = 0; j < (*goalTraitDistribution)[0].size(); j++)
            {
                float dist = (*goalTraitDistribution)[i][j] - allocationTraitDistribution[i][j];
                if(dist > 0)
                {
                    goalDistance += dist;
                }
            }
        }
        if(goalDistance <= epsilon)
        {
            goalDistance = 0;
        }
        isGoal       = checkGoalAllocation();
        scheduleTime = -1;
    }

    boost::shared_ptr<vector<float>> TaskAllocation::getActionDuration()
    {
        return actionDurations;
    }

    void TaskAllocation::setActionDuration(boost::shared_ptr<vector<float>> newActionDur)
    {
        actionDurations = newActionDur;
        scheduleTime    = -1;
    }

    boost::shared_ptr<vector<vector<int>>> TaskAllocation::getOrderingConstraints()
    {
        return orderingConstraints;
    }

    void TaskAllocation::setOrderingConstraints(boost::shared_ptr<vector<vector<int>>> newOrderingCon)
    {
        orderingConstraints = newOrderingCon;
        scheduleTime        = -1;
    }

    float TaskAllocation::getScheduleTime()
    {
        if(scheduleTime > 0)
        {
            return scheduleTime;
        }
        else
        {
            if(!usingSpecies)
            {
                scheduleTime = taToScheduling.getNonSpeciesSchedule(this);
            }
            else
            {
                scheduleTime = taToScheduling.getSpeciesSchedule(this);
            }
            return scheduleTime;
        }
    }

    void TaskAllocation::addAction(const vector<float>& actionRequirements,
                                   const vector<float>& noncumTraitCutoff,
                                   const float newActionDuration,
                                   vector<vector<int>>* orderingCon)
    {
        if(newActionDuration <= 0)
        {
            actionDurations->push_back(newActionDuration);
            orderingConstraints->insert(orderingConstraints->end(), orderingCon->begin(), orderingCon->end());
        }
        goalTraitDistribution->push_back(actionRequirements);
        actionNoncumulativeTraitValue->push_back(noncumTraitCutoff);

        vector<int> emptyVect(speciesTraitDistribution->size(), 0.0);
        allocation.insert(allocation.end(), emptyVect.begin(), emptyVect.end());
        if(isGoal)
        {
            isGoal = checkGoalAllocation();
        }
        for(float actionRequirement: actionRequirements)
        {
            goalDistance += actionRequirement;
        }
        scheduleTime = -1;
    }

    void TaskAllocation::addAction(const vector<float>& actionRequirements,
                                   const vector<float>& nonCumActionRequirements,
                                   float goalDistAdd,
                                   const float& newActionDuration,
                                   vector<vector<int>>* orderingCon)
    {
        goalTraitDistribution->push_back(actionRequirements);
        actionNoncumulativeTraitValue->push_back(nonCumActionRequirements);
        allocationTraitDistribution.emplace_back(this->speciesTraitDistribution->size(), 0);
        if(newActionDuration <= 0)
        {
            actionDurations->push_back(newActionDuration);
            orderingConstraints->insert(orderingConstraints->end(), orderingCon->begin(), orderingCon->end());
        }

        vector<int> emptyVect(speciesTraitDistribution->size(), 0.0);
        allocation.insert(allocation.end(), emptyVect.begin(), emptyVect.end());

        goalDistance += goalDistAdd;
        if(isGoal){
            isGoal = checkGoalAllocation();
        }
        scheduleTime = -1;
    }

}  // namespace grstaps
