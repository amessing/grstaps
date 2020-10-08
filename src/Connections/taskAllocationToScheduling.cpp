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
#include <math.h>       /* pow */

namespace grstaps
{
    taskAllocationToScheduling::taskAllocationToScheduling(boost::shared_ptr<std::vector<boost::shared_ptr<MotionPlanner>>> motion_planners,
                                                           const std::vector<unsigned int>* startingLoc,
                                                           float longestMotion)
        : m_motion_planners(motion_planners)
        , m_starting_locations(startingLoc)
        , longestMP(longestMotion)
    {}

    float taskAllocationToScheduling::getNonSpeciesSchedule(TaskAllocation* allocObject)
    {

        std::vector<std::vector<int>> disjunctiveConstraints;
        int numAction = allocObject->allocation.size() / (*allocObject->getNumSpecies()).size();

        for(int species = 0; species < (*allocObject->getNumSpecies()).size(); ++species)
        {
            concurrent.clear();

            for(int action = 0; action < numAction; ++action)
            {
                if(allocObject->allocation[(*allocObject->getNumSpecies()).size() * action + species] > 0)
                {

                    for(int concur = 0; concur < concurrent.size(); ++concur)
                    {
                        vector<int> constraint = {concurrent[concur], action};
                        auto found = std::find(disjunctiveConstraints.begin(), disjunctiveConstraints.end(), constraint);
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
               *allocObject->getActionDuration(), *allocObject->getOrderingConstraints(), disjunctiveConstraints, longestMP))
        {
            adjustScheduleNonSpeciesSchedule(allocObject);

            return addMotionPlanningNonSpeciesSchedule(allocObject);
        }
        return -1;
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
        Timer mpTime;
        mpTime.start();

        if(m_motion_planners == nullptr)
        {
            mpTime.recordSplit(Timer::SplitType::e_mp);
            mpTime.stop();
            return sched.getMakeSpan();
        }
        else
        {

            std::vector<unsigned int> currentLocations = *m_starting_locations;
            for(int i = 0; i < actionOrder.size(); ++i)
            {

                float maxTravelTime           = 0;
                vector<vector<float>>* traits = TaskAlloc->getSpeciesTraitDistribution();
                float slowestAgent            = TaskAlloc->maxSpeed;
                int slowestAgentIndex = -1;

                for(int j = 0; j < TaskAlloc->getNumSpecies()->size(); j++)
                {
                    if(TaskAlloc->allocation[actionOrder[i] * TaskAlloc->getNumSpecies()->size() + j] == 1)
                    {
                        if(TaskAlloc->speedIndex == -1)
                        {
                            slowestAgent = 1;
                            slowestAgentIndex = 0;}
                        else{
                            if(slowestAgent >= (*traits)[j][TaskAlloc->speedIndex])
                            {
                                slowestAgent = (*traits)[j][TaskAlloc->speedIndex];
                                slowestAgentIndex = j;
                            }
                        }
                        if(currentLocations[j] != (*m_action_locations)[actionOrder[i]].first)
                        {
                            std::pair<bool, float> travelTime =
                                (*m_motion_planners)[(*TaskAlloc->speciesTraitDistribution)[j][TaskAlloc->mp_Index]]->query(currentLocations[j], (*m_action_locations)[actionOrder[i]].first);
                            if(travelTime.first)
                            {
                                if(TaskAlloc->speedIndex == -1)
                                {
                                    slowestAgent = 1;
                                    slowestAgentIndex = 0;
                                    if((travelTime.second) > maxTravelTime)
                                    {
                                        maxTravelTime = travelTime.second;
                                        // Move to the end of the action
                                    }
                                    currentLocations[j] = (*m_action_locations)[actionOrder[i]].second;
                                }
                                else
                                {
                                    if((travelTime.second / (*traits)[j][TaskAlloc->speedIndex]) > maxTravelTime)
                                    {
                                        maxTravelTime = travelTime.second / (*traits)[j][TaskAlloc->speedIndex];
                                        // Move to the end of the action
                                    }
                                    currentLocations[j] = (*m_action_locations)[actionOrder[i]].second;
                                }
                            }
                            else
                            {
                                return -1;
                            }
                        }
                    }
                }

                // If the action is a move action then add in the move time
                float action_move_time = 0.0;
                if((*m_action_locations)[actionOrder[i]].first != (*m_action_locations)[actionOrder[i]].second)
                {
                    std::pair<bool, float> action_travel_length;
                    if(slowestAgentIndex != -1)
                    {

                        action_travel_length =
                            (*m_motion_planners)[(*TaskAlloc->speciesTraitDistribution)[slowestAgentIndex][TaskAlloc->mp_Index]]->query(
                                (*m_action_locations)[actionOrder[i]].first,
                                (*m_action_locations)[actionOrder[i]].second);
                    }
                    else{
                        Location& first = (*m_motion_planners)[0]->m_locations[(*m_action_locations)[actionOrder[i]].first];
                        Location& second= (*m_motion_planners)[0]->m_locations[(*m_action_locations)[actionOrder[i]].second];

                        float x_dist = first.x() - second.x();
                        float y_dist = first.y() - second.y();

                        action_travel_length = {true, sqrt( pow(x_dist,2) + pow(y_dist,2))};
                    }

                    if(action_travel_length.first)
                    {
                        action_move_time = action_travel_length.second;
                    }
                        // The movement required during action i is impossible
                    else
                    {
                        return -1;
                    }
                }

                sched.increaseActionTime(actionOrder[i], maxTravelTime + (action_move_time / slowestAgent));
            }
            mpTime.recordSplit(Timer::SplitType::e_mp);
            mpTime.stop();
            return sched.getMakeSpan();
        }
    }

    std::pair<bool, vector<agent_motion_plans>> taskAllocationToScheduling::saveMotionPlanningNonSpeciesSchedule(
        TaskAllocation* TaskAlloc)
    {
        std::vector<agent_motion_plans> motionPlans(TaskAlloc->getNumSpecies()->size());

        if(m_motion_planners == nullptr)
        {
            return std::pair<bool, vector<agent_motion_plans>>(false, motionPlans);
        }
        else
        {
            sched.actionStartTimes = vector<float>(sched.stn.size(),0);
            std::vector<unsigned int> currentLocations = *m_starting_locations;
            for(int i = 0; i < actionOrder.size(); ++i)
            {
                auto traits = TaskAlloc->getSpeciesTraitDistribution();
                for(int j = 0; j < TaskAlloc->getNumSpecies()->size(); j++)
                {
                    if(TaskAlloc->allocation[actionOrder[i] * TaskAlloc->getNumSpecies()->size() + j] == 1)
                    {
                        if(currentLocations[j] != (*m_action_locations)[actionOrder[i]].first)
                        {
                            std::tuple<bool, float, std::vector<std::pair<float, float>>> waypoints =
                                (*m_motion_planners)[(*TaskAlloc->speciesTraitDistribution)[j][TaskAlloc->mp_Index]]->getWaypoints(currentLocations[j], (*m_action_locations)[actionOrder[i]].first);

                            if(std::get<0>(waypoints))
                            {
                                float travel_time = std::get<1>(waypoints);
                                if(TaskAlloc->speedIndex != -1)
                                {
                                    travel_time /= (*traits)[j][TaskAlloc->speedIndex];
                                }
                                if( (sched.stn[actionOrder[i]][0] + travel_time) > sched.actionStartTimes[actionOrder[i]]){
                                    sched.actionStartTimes[actionOrder[i]] = sched.stn[actionOrder[i]][0] + travel_time;
                                }

                                currentLocations[j] = (*m_action_locations)[actionOrder[i]].second;
                                start_end time      = {sched.stn[i][0], sched.stn[i][0] + travel_time};
                                single_plan step    = std::make_pair(time, std::get<2>(waypoints));
                                motionPlans[j].push_back(step);
                            }
                            else
                            {
                                return std::pair<bool, std::vector<agent_motion_plans>>(
                                    false, std::vector<agent_motion_plans>());
                            }
                        }

                        // If the action is a move action then add in the move time
                        if((*m_action_locations)[actionOrder[i]].first != (*m_action_locations)[actionOrder[i]].second)
                        {
                            std::tuple<bool, float, std::vector<std::pair<float, float>>> waypoints =
                                (*m_motion_planners)[(*TaskAlloc->speciesTraitDistribution)[j][TaskAlloc->mp_Index]]->getWaypoints((*m_action_locations)[actionOrder[i]].first,
                                                               (*m_action_locations)[actionOrder[i]].second);
                            if(std::get<0>(waypoints))
                            {
                                // float action_duration_half = (*TaskAlloc->actionDurations)[actionOrder[i]] / 2.0;
                                // start_end  time = {stn[i][0] + action_duration_half, stn[i][1] -
                                // action_duration_half};
                                float travel_time = std::get<1>(waypoints);
                                float adj_travel_time = travel_time;
                                if(TaskAlloc->speedIndex != -1)
                                {
                                    adj_travel_time = 0;
                                    for(int k=0; k <  TaskAlloc->getNumSpecies()->size(); ++k){
                                        if(TaskAlloc->allocation[actionOrder[i] * TaskAlloc->getNumSpecies()->size() + k] == 1)
                                        {
                                            if(adj_travel_time < (travel_time/(*traits)[k][TaskAlloc->speedIndex])){
                                                adj_travel_time =  (travel_time/(*traits)[k][TaskAlloc->speedIndex]);
                                            }
                                        }
                                    }
                                }

                                start_end time   = {sched.stn[i][1] - adj_travel_time - (*TaskAlloc->actionDurations)[i], sched.stn[i][1]};
                                if(time.first < 0.00001){
                                    time.first = 0;
                                }

                                /*
                                if(i == sched.actionStartTimes.size()){
                                    sched.actionStartTimes.push_back(sched.stn[i][1] - adj_travel_time - (*TaskAlloc->actionDurations)[i]);
                                    if(sched.stn[i][1] - adj_travel_time - (*TaskAlloc->actionDurations)[i] < 0.00001){
                                        sched.actionStartTimes[ sched.actionStartTimes.size()-1] = 0;
                                    }
                                }
                                */

                                single_plan step = std::make_pair(time, std::get<2>(waypoints));
                                motionPlans[j].push_back(step);
                            }
                        }
                        // The movement required during action i is impossible
                        else
                        {
                            start_end time   = {sched.stn[i][1] - (*TaskAlloc->actionDurations)[i], sched.stn[i][1]};
                            std::vector<std::pair<float, float>> waypoints = std::vector<std::pair<float, float>>();
                            single_plan step = std::make_pair(time, waypoints);
                            motionPlans[j].push_back(step);
                            //    return std::pair<bool, vector<agent_motion_plans>>(false, motionPlans);
                        }
                    }
                }
            }
        }
        return std::make_pair(true, motionPlans);
    }

    void taskAllocationToScheduling::setActionLocations(
        boost::shared_ptr<const std::vector<std::pair<unsigned int, unsigned int>>> action_locations)
    {
        m_action_locations = std::move(action_locations);
    }
}  // namespace grstaps
