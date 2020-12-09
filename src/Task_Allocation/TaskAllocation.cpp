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
#include <grstaps/Connections/taskAllocationToScheduling.h>

namespace grstaps
{
    float epsilon = 0.0001;

    TaskAllocation::TaskAllocation(bool useSpec,
                                   std::shared_ptr<vector<vector<float>>> goalDistribution,
                                   vector<vector<float>>* speciesDistribution,
                                   vector<short> startAllocation,
                                   std::shared_ptr<vector<vector<float>>> noncumTraitCutoff,
                                   taskAllocationToScheduling taToSched,
                                   std::shared_ptr<vector<float>> actionDur,
                                   std::shared_ptr<vector<vector<int>>> orderingCon,
                                   std::shared_ptr<vector<int>> numSpec,
                                   int speedInd,
                                   int mpIndex)
    {
        action_dynamics               = vector<short>(goalDistribution->size(),-1);
        taToScheduling                = std::move(taToSched);
        goalDistance                  = 0.0;
        allocation                    = std::move(startAllocation);

        requirementsRemaining         = *goalDistribution;
        scheduleTime                  = -1;
        allocationTraitDistribution = vector<vector<float>>(goalDistribution->size(), std::vector<float>((*goalDistribution)[0].size(), 0));

        params = std::make_shared<taskAllocationSearchParams>(
                                                                useSpec,
                                                                goalDistribution,
                                                                speciesDistribution,
                                                                noncumTraitCutoff,
                                                                actionDur,
                                                                orderingCon,
                                                                numSpec,
                                                                speedInd,
                                                                mpIndex);
        updateAllocationTraitDistribution();
        params->startingGoalDistance = goalDistance;
        isGoal = checkGoalAllocation();

    }

    TaskAllocation::TaskAllocation(bool useSpec,
                                   std::shared_ptr<vector<vector<float>>> goalDistribution,
                                   vector<vector<float>>* speciesDistribution,
                                   std::shared_ptr<vector<vector<float>>> noncumTraitCutoff,
                                   taskAllocationToScheduling taToSched,
                                   std::shared_ptr<vector<float>> actionDur,
                                   std::shared_ptr<vector<vector<int>>> orderingCon,
                                   std::shared_ptr<vector<int>> numSpec,
                                   int speedInd,
                                   int mpIndex)
    {
        action_dynamics               = vector<short>(goalDistribution->size(),-1);
        taToScheduling                = std::move(taToSched);
        requirementsRemaining         = *goalDistribution;
        goalDistance                  = 0.0;
        allocation.resize(goalDistribution->size() * speciesDistribution->size(), 0);
        scheduleTime = -1;


        allocationTraitDistribution = vector<vector<float>>(goalDistribution->size(), std::vector<float>((*goalDistribution)[0].size(), 0));

        params = std::make_shared<taskAllocationSearchParams>(
                                                                useSpec,
                                                                goalDistribution,
                                                                speciesDistribution,
                                                                noncumTraitCutoff,
                                                                actionDur,
                                                                orderingCon,
                                                                numSpec,
                                                                speedInd,
                                                                mpIndex);
        updateAllocationTraitDistribution();
        isGoal = checkGoalAllocation();
        params->startingGoalDistance = goalDistance;


    }

    TaskAllocation::TaskAllocation(const TaskAllocation& copyAllocation)
    {

        taToScheduling              = copyAllocation.taToScheduling;
        scheduleTime                = copyAllocation.scheduleTime;
        goalDistance                = copyAllocation.goalDistance;
        isGoal                      = copyAllocation.isGoal;
        allocation                  = copyAllocation.allocation;
        allocationTraitDistribution = copyAllocation.allocationTraitDistribution;
        requirementsRemaining       = copyAllocation.requirementsRemaining;
        action_dynamics             = copyAllocation.action_dynamics;
        params                      = copyAllocation.params;

    }

    bool TaskAllocation::checkGoalAllocation() const
    {
        return goalDistance <= epsilon;
    }

    std::shared_ptr<vector<int>> TaskAllocation::getNumSpecies()
    {
        return params->numSpecies;
    }

    const std::vector<short>& TaskAllocation::getAllocation() const
    {
        return allocation;
    }

    float TaskAllocation::getScheduleTime()
    {
        if(scheduleTime > 0)
        {
            return scheduleTime;
        }
        else
        {
            if(!params->usingSpecies)
            {
                scheduleTime = taToScheduling.getNonSpeciesSchedule(this);
            }
            else
            {
                scheduleTime = grstaps::taskAllocationToScheduling::getSpeciesSchedule(this);
            }
            return scheduleTime;
        }
    }

    vector<vector<float>>* TaskAllocation::getSpeciesTraitDistribution()
    {
        return params->speciesTraitDistribution;
    }

    std::string TaskAllocation::getID()
    {
        std::string ID;
        int largestDigit = 1;
        for(int i = 0; i < params->numSpecies->size(); i++)
        {
            int digits    = 0;
            int numAgents = (*params->numSpecies)[i];
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

    bool TaskAllocation::addAgent(int agentIndex, int taskIndex)
    {
        bool added = false;
        if(allocation[taskIndex * params->speciesTraitDistribution->size() + agentIndex] < (*params->numSpecies)[agentIndex])
        {
            allocation[taskIndex * params->speciesTraitDistribution->size() + agentIndex] += 1;
            updateAllocationTraitDistributionAgent(agentIndex, taskIndex);
            added  = true;
            isGoal = checkGoalAllocation();
            action_dynamics[taskIndex] = (*params->speciesTraitDistribution)[agentIndex][params->mp_Index];
        }
        scheduleTime = -1;
        return added;
    }

    // todo update
    void TaskAllocation::updateAllocationTraitDistributionAgent(int agentIndex, int taskIndex)
    {
        for(int i = 0; i < (*params->speciesTraitDistribution)[0].size(); i++)
        {
            float allocBefore = allocationTraitDistribution[taskIndex][i];
            allocationTraitDistribution[taskIndex][i] += ((*params->speciesTraitDistribution)[agentIndex][i]);
            if((*params->actionNoncumulativeTraitValue)[taskIndex][i] != 0.0)
            {
                if((*params->speciesTraitDistribution)[agentIndex][i] >= (*params->actionNoncumulativeTraitValue)[taskIndex][i])
                {
                    if(allocBefore < (*params->goalTraitDistribution)[taskIndex][i])
                    {
                        goalDistance -= 1;
                        requirementsRemaining[taskIndex][i] -= 1;
                    }
                }
            }
            else
            {
                if(allocBefore < (*params->goalTraitDistribution)[taskIndex][i])
                {
                    float allocAfter = allocBefore + ((*params->speciesTraitDistribution)[agentIndex][i]);
                    if(allocAfter < (*params->goalTraitDistribution)[taskIndex][i])
                    {
                        goalDistance -= ((*params->speciesTraitDistribution)[agentIndex][i]);
                        requirementsRemaining[taskIndex][i] -= ((*params->speciesTraitDistribution)[agentIndex][i]);
                    }
                    else
                    {
                        goalDistance -= (*params->goalTraitDistribution)[taskIndex][i] - allocBefore;
                        requirementsRemaining[taskIndex][i] -= (*params->goalTraitDistribution)[taskIndex][i] - allocBefore;
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
        allocationTraitDistribution.resize(params->goalTraitDistribution->size(),
                                           vector<float>((*params->goalTraitDistribution)[0].size(), 0.0));
        for(int i = 0; i < allocationTraitDistribution.size(); i++)
        {
            for(int j = 0; j < allocationTraitDistribution[0].size(); j++)
            {
                for(int k = 0; k < params->speciesTraitDistribution->size(); k++)
                {
                    if((*params->actionNoncumulativeTraitValue)[i][j] != 0.0)
                    {
                        if((*params->speciesTraitDistribution)[k][j] >= (*params->actionNoncumulativeTraitValue)[i][j])
                        {
                            allocationTraitDistribution[i][j] += allocation[i * params->speciesTraitDistribution->size() + k];
                        }
                        if(allocationTraitDistribution[i][j] > (*params->goalTraitDistribution)[i][j])
                        {
                            allocationTraitDistribution[i][j] = (*params->goalTraitDistribution)[i][j];
                        }
                    }
                    else
                    {
                        allocationTraitDistribution[i][j] += ((*params->speciesTraitDistribution)[k][j] * allocation[i * params->speciesTraitDistribution->size() + k]);
                        if(allocationTraitDistribution[i][j] > (*params->goalTraitDistribution)[i][j])
                        {
                            allocationTraitDistribution[i][j] = (*params->goalTraitDistribution)[i][j];
                        }
                    }
                }
            }
        }
        for(int i = 0; i < (*params->goalTraitDistribution).size(); i++)
        {
            for(int j = 0; j < (*params->goalTraitDistribution)[0].size(); j++)
            {
                float dist = (*params->goalTraitDistribution)[i][j] - allocationTraitDistribution[i][j];
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

    std::shared_ptr<vector<float>> TaskAllocation::getActionDuration()
    {
        return params->actionDurations;
    }

    std::shared_ptr<vector<vector<int>>> TaskAllocation::getOrderingConstraints()
    {
        return params->orderingConstraints;
    }

    [[maybe_unused]] void TaskAllocation::setNumSpecies(std::shared_ptr<vector<int>> newNumSpecies)
    {
        params->numSpecies = std::move(newNumSpecies);
    }

    [[maybe_unused]] void TaskAllocation::checkSize()
    {
        std::cout << " 1) Scheduler* taToSchedule{};= " << sizeof(taToScheduling) << std::endl;
        std::cout << " 2) float scheduleTime{};= " << sizeof(scheduleTime) << std::endl;
        std::cout << " 3) float goalDistance{};= " << sizeof(goalDistance) << std::endl;
        std::cout << " 4) bool isGoal{};= " << sizeof(isGoal) << std::endl;
        std::cout << " 5) vector<short> allocation;= " << (sizeof(std::vector<short>) + (sizeof(short) * allocation.size())) << std::endl;
        std::cout << " 6) vector<vector<float>> requirementsRemaining;= " << ((requirementsRemaining.size()+1) * sizeof(std::vector<float>) + (requirementsRemaining.size() * sizeof(float) * requirementsRemaining[0].size())) << std::endl;
        std::cout << " 7) vector<vector<float>> allocationTraitDistribution;= " <<  ((allocationTraitDistribution.size()+1) * sizeof(std::vector<float>) + (allocationTraitDistribution.size() * sizeof(float) * allocationTraitDistribution[0].size())) << std::endl;
        std::cout << " 8) std::shared_ptr<taskAllocationSearchParams> params; " << sizeof(params) << std::endl;
        std::cout << " 9) vector<int> action_dynamics; " << (sizeof(std::vector<short>) + (sizeof(short) * action_dynamics.size())) << std::endl;
    }

    [[maybe_unused]] void TaskAllocation::setActionDuration(std::shared_ptr<vector<float>> newActionDur)
    {
        params->actionDurations = std::move(newActionDur);
        scheduleTime    = -1;
    }

    [[maybe_unused]] void TaskAllocation::setOrderingConstraints(std::shared_ptr<vector<vector<int>>> newOrderingCon)
    {
        params->orderingConstraints = std::move(newOrderingCon);
        scheduleTime        = -1;
    }

    [[maybe_unused]] void TaskAllocation::addAction(const vector<float>& actionRequirements,
                                   const vector<float>& noncumTraitCutoff,
                                   const float newActionDuration,
                                   vector<vector<int>>* orderingCon)
    {
        if(newActionDuration <= 0)
        {
            params->actionDurations->push_back(newActionDuration);
            params->orderingConstraints->insert(params->orderingConstraints->end(), orderingCon->begin(), orderingCon->end());
        }
        params->goalTraitDistribution->push_back(actionRequirements);
        params->actionNoncumulativeTraitValue->push_back(noncumTraitCutoff);

        vector<int> emptyVect(params->speciesTraitDistribution->size(), 0.0);
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

    [[maybe_unused]] void TaskAllocation::addAction(const vector<float>& actionRequirements,
                                   const vector<float>& nonCumActionRequirements,
                                   float goalDistAdd,
                                   const float& newActionDuration,
                                   vector<vector<int>>* orderingCon)
    {
        params->goalTraitDistribution->push_back(actionRequirements);
        params->actionNoncumulativeTraitValue->push_back(nonCumActionRequirements);
        allocationTraitDistribution.emplace_back(params->speciesTraitDistribution->size(), 0);
        if(newActionDuration <= 0)
        {
            params->actionDurations->push_back(newActionDuration);
            params->orderingConstraints->insert(params->orderingConstraints->end(), orderingCon->begin(), orderingCon->end());
        }

        vector<int> emptyVect(params->speciesTraitDistribution->size(), 0.0);
        allocation.insert(allocation.end(), emptyVect.begin(), emptyVect.end());

        goalDistance += goalDistAdd;
        if(isGoal){
            isGoal = checkGoalAllocation();
        }
        scheduleTime = -1;
    }

    [[maybe_unused]] void TaskAllocation::setAllocation(const std::vector<short>& newAllocation)
    {
        allocation = newAllocation;
        updateAllocationTraitDistribution();
    }

    [[maybe_unused]] void TaskAllocation::setGoalTraitDistribution(
        std::shared_ptr<vector<vector<float>>> newGoalTraitDistribution)
    {
        params->goalTraitDistribution = std::move(newGoalTraitDistribution);
        isGoal                = checkGoalAllocation();
        updateAllocationTraitDistribution();
    }

    [[maybe_unused]] void TaskAllocation::setSpeciesTraitDistribution(
        vector<vector<float>>* newSpeciesTraitDistribution)
    {
        params->speciesTraitDistribution = newSpeciesTraitDistribution;
        updateAllocationTraitDistribution();
    }

    [[maybe_unused]] void TaskAllocation::setActionNoncumulativeTraitValue(
        std::shared_ptr<vector<vector<float>>> newActionNoncumulativeTraitValue)
    {
        params->actionNoncumulativeTraitValue = std::move(newActionNoncumulativeTraitValue);
        updateAllocationTraitDistribution();
    }

    [[maybe_unused]] const vector<vector<float>>& TaskAllocation::getAllocationTraitDistribution() const
    {
        return allocationTraitDistribution;
    }

    [[maybe_unused]] std::shared_ptr<vector<vector<float>>> TaskAllocation::getGoalTraitDistribution() const
    {
        return params->goalTraitDistribution;
    }

    [[maybe_unused]] std::shared_ptr<vector<vector<float>>> TaskAllocation::getActionNoncumulativeTraitValue()
    {
        return params->actionNoncumulativeTraitValue;
    }

    [[maybe_unused]] std::shared_ptr<vector<int>> TaskAllocation::getnumSpecies()
    {
        return params->numSpecies;
    }
}  // namespace grstaps
