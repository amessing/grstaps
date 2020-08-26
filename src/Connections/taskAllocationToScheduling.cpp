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

#include "grstaps/Connections/taskAllocationToScheduling.h"

#include "grstaps/Task_Allocation/TaskAllocation.h"
#include "grstaps/logger.hpp"
#include "grstaps/motion_planning/motion_planner.hpp"

namespace grstaps
{
    taskAllocationToScheduling::taskAllocationToScheduling(MotionPlanner* motion_planner,
                                                           const std::vector<unsigned int>& startingLoc)
    {
        m_motion_planner     = motion_planner;
        m_starting_locations = startingLoc;
    }

    float taskAllocationToScheduling::getNonSpeciesSchedule(TaskAllocation* allocObject)
    {
        std::vector<std::vector<int>> disjunctiveConstraints;
        for(int species = 0; species < (*allocObject->getNumSpecies()).size(); ++species)
        {
            concurrent.clear();

            int numAction = allocObject->allocation.size() / (*allocObject->getNumSpecies()).size();
            for(int action = 0; action < numAction; ++action)
            {
                if(allocObject->allocation[(*allocObject->getNumSpecies()).size() * action + species] > 0)
                {
                    for(int concur = 0; concur < concurrent.size(); ++concur)
                    {
                        vector<int> constraint = {concurrent[concur], action};
                        auto found =
                            std::find(disjunctiveConstraints.begin(), disjunctiveConstraints.end(), constraint);
                        if(found == disjunctiveConstraints.end())
                        {
                            disjunctiveConstraints.push_back(constraint);
                        }
                    }
                    concurrent.push_back(action);
                }
            }
        }
        if(sched.schedule(
               *allocObject->getActionDuration(), *allocObject->getOrderingConstraints(), disjunctiveConstraints))
        {
            adjustScheduleNonSpeciesSchedule(allocObject);
            return addMotionPlanningNonSpeciesSchedule(allocObject);
        }
        return -10;
    }

    float taskAllocationToScheduling::getSpeciesSchedule(TaskAllocation* allocObject)
    {
        throw "Not implemented yet.";
    }

    void taskAllocationToScheduling::adjustScheduleNonSpeciesSchedule(TaskAllocation* taskAlloc)
    {
        actionOrder.clear();
        // auto allocation = taskAlloc->getAllocation();
        stn = sched.stn;
        vector<int> checked(sched.stn.size(), 0);

        for(int i = 0; i < stn.size(); ++i)
        {
            vector<vector<float>> stn = sched.stn;

            // find action ending soonest

            float minEndTime      = std::numeric_limits<float>::max();
            int currentSoonestEnd = 0;
            for(int j = 0; j < stn.size(); ++j)
            {
                if(stn[j][1] < minEndTime && !checked[j])
                {
                    minEndTime        = stn[j][1];
                    currentSoonestEnd = j;
                }
            }
            actionOrder.emplace_back(currentSoonestEnd);

            // find concurrent actionsector<int> concurrent{currentSoonestEnd};
            concurrent.clear();
            concurrent.emplace_back(currentSoonestEnd);
            for(int j = 0; j < stn.size(); ++j)
            {
                if((j != currentSoonestEnd) && !checked[j] &&
                   ((stn[j][1] <= stn[currentSoonestEnd][0] && stn[j][1] > stn[currentSoonestEnd][0]) ||
                    (stn[j][0] <= stn[currentSoonestEnd][1] && stn[j][0] >= stn[currentSoonestEnd][0])))
                {
                    concurrent.emplace_back(j);
                }
            }

            // calc trait ussage at time of action
            maxTraitTeam = *(taskAlloc->traitTeamMax);
            for(int j = 0; j < concurrent.size(); ++j)
            {
                for(int k = 0; k < maxTraitTeam.size(); ++k)
                {
                    maxTraitTeam[k] -= taskAlloc->allocationTraitDistribution[concurrent[j]][k] +
                                       taskAlloc->requirementsRemaining[concurrent[j]][k];
                }
            }

            bool removeCurrent = true;
            for(int j = 0; j < maxTraitTeam.size(); ++j)
            {
                while(maxTraitTeam[j] < 0)
                {
                    stn                 = sched.stn;
                    int toUpdate        = 1000;
                    int direction       = 0;
                    float bestSchedTime = std::numeric_limits<float>::max();
                    float currentSched  = std::numeric_limits<float>::max();

                    for(int k = 0; k < concurrent.size(); ++k)
                    {
                        for(int l = 0; l < 2; ++l)
                        {
                            if(((*taskAlloc->goalTraitDistribution)[concurrent[k]][j] > 0) ||
                               (taskAlloc->allocationTraitDistribution[concurrent[k]][j] > 0))
                            {
                                if(concurrent[k] != currentSoonestEnd)
                                {
                                    if(l == 0)
                                    {
                                        currentSched = sched.addOCTemp(currentSoonestEnd,
                                                                       concurrent[k],
                                                                       stn,
                                                                       sched.beforeConstraints,
                                                                       sched.afterConstraints);

                                        if(currentSched < bestSchedTime)
                                        {
                                            bestSchedTime = currentSched;
                                            toUpdate      = k;
                                            direction     = l;
                                        }
                                    }
                                    else
                                    {
                                        currentSched = sched.addOCTemp(currentSoonestEnd,
                                                                       concurrent[k],
                                                                       stn,
                                                                       sched.beforeConstraints,
                                                                       sched.afterConstraints);

                                        if(currentSched < bestSchedTime)
                                        {
                                            bestSchedTime = currentSched;
                                            toUpdate      = k;
                                            direction     = l;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if(direction == 0)
                    {
                        sched.addOC(currentSoonestEnd, concurrent[toUpdate]);

                        for(int k = 0; k < (*taskAlloc->goalTraitDistribution)[concurrent[toUpdate]].size(); ++k)
                        {
                            maxTraitTeam[k] += taskAlloc->allocationTraitDistribution[concurrent[toUpdate]][k] +
                                               taskAlloc->requirementsRemaining[concurrent[toUpdate]][k];
                        }
                        concurrent.erase(concurrent.begin() + toUpdate);
                    }
                    else
                    {
                        sched.addOC(concurrent[toUpdate], currentSoonestEnd);
                        concurrent.erase(concurrent.begin() + toUpdate);
                        j             = maxTraitTeam.size();
                        removeCurrent = false;
                        break;
                    }
                }
            }

            if(removeCurrent)
            {
                checked[currentSoonestEnd] = 1;
            }
        }
        // return sched.getMakeSpan();
    }

    float taskAllocationToScheduling::addMotionPlanningNonSpeciesSchedule(TaskAllocation* TaskAlloc)
    {
        if(m_motion_planner == nullptr)
        {
            return sched.getMakeSpan();
        }
        else
        {
            std::vector<unsigned int> currentLocations = m_starting_locations;
            for(int i = 0; i < actionOrder.size(); ++i)
            {
                // If the action is a move action then add in the move time
                float action_move_time = 0.0;
                if(m_action_locations[actionOrder[i]].first != m_action_locations[actionOrder[i]].second)
                {
                    Logger::debug("MP Query {} -> {}",
                                  m_action_locations[actionOrder[i]].first,
                                  m_action_locations[actionOrder[i]].second);
                    std::pair<bool, float> action_travel_length = m_motion_planner->query(
                        m_action_locations[actionOrder[i]].first, m_action_locations[actionOrder[i]].second);
                    if(action_travel_length.first)
                    {
                        action_move_time = action_travel_length.second;
                    }
                    // The movement required during action i is impossible
                    else
                    {
                        return std::numeric_limits<float>::max();
                    }
                }

                float maxTravelTime = 0;
                float slowestAgent  = std::numeric_limits<float>::max();
                auto traits         = TaskAlloc->getSpeciesTraitDistribution();
                for(int j = 0; j < TaskAlloc->getNumSpecies()->size(); j++)
                {
                    if(TaskAlloc->allocation[actionOrder[i] * TaskAlloc->getNumSpecies()->size() + j] == 1)
                    {
                        if(currentLocations[j] != m_action_locations[actionOrder[i]].first)
                        {
                            Logger::debug(
                                "MP Query {} -> {}", currentLocations[j], m_action_locations[actionOrder[i]].first);
                            std::pair<bool, float> travelTime =
                                m_motion_planner->query(currentLocations[j], m_action_locations[actionOrder[i]].first);
                            if(travelTime.first)
                            {
                                if(TaskAlloc->speedIndex == -1)
                                {
                                    slowestAgent = 1;
                                    if((travelTime.second) > maxTravelTime)
                                    {
                                        maxTravelTime = travelTime.second;
                                        // Move to the end of the action
                                        currentLocations[j] = m_action_locations[actionOrder[i]].second;
                                    }
                                }
                                else
                                {
                                    if(slowestAgent < (*traits)[j][TaskAlloc->speedIndex])
                                    {
                                        slowestAgent = (*traits)[j][TaskAlloc->speedIndex];
                                    }
                                    if((travelTime.second / (*traits)[j][TaskAlloc->speedIndex]) > maxTravelTime)
                                    {
                                        maxTravelTime = travelTime.second / (*traits)[j][TaskAlloc->speedIndex];
                                        // Move to the end of the action
                                        currentLocations[j] = m_action_locations[actionOrder[i]].second;
                                    }
                                }
                            }
                            else
                            {
                                return std::numeric_limits<float>::max();
                            }
                        }
                    }
                }
                sched.increaseActionTime(actionOrder[i], maxTravelTime + (action_move_time / slowestAgent));
            }
            return sched.getMakeSpan();
        }
    }

    vector<vector<float>> taskAllocationToScheduling::saveMotionPlanningNonSpeciesSchedule(TaskAllocation* TaskAlloc)
    {
        std::vector<std::vector<float>> motionPlans(TaskAlloc->getNumSpecies()->size(), std::vector<float>(1, 0));
        if(m_motion_planner != nullptr)
        {
            // Why is this vector created?
            std::vector<unsigned int> currentLocations = m_starting_locations;
            for(int i = 0; i < currentLocations.size(); ++i)
            {
                motionPlans[i][0] = currentLocations[i];
            }
            for(int i = 0; i < actionOrder.size(); ++i)
            {
                for(int j = 0; TaskAlloc->getNumSpecies()->size(); j++)
                {
                    if(TaskAlloc->allocation[actionOrder[i] * TaskAlloc->getNumSpecies()->size() + j] == 1)
                    {
                        motionPlans[j].emplace_back(m_action_locations[actionOrder[i]].first);
                        if(m_action_locations[actionOrder[i]].second != m_action_locations[actionOrder[i]].first)
                        {
                            motionPlans[j].emplace_back(m_action_locations[actionOrder[i]].second);
                        }
                    }
                }
            }
        }
        return motionPlans;
    }

    void taskAllocationToScheduling::setActionLocations(
        const std::vector<std::pair<unsigned int, unsigned int>>& action_locations)
    {
        m_action_locations = action_locations;
    }
}  // namespace grstaps
