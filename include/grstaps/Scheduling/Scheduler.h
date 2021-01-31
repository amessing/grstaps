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
#ifndef GRSTAPS_SCHEDULER_H
#define GRSTAPS_SCHEDULER_H

// external
#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <../lib/unordered_map/robin_hood.h>
#include <boost/heap/binomial_heap.hpp>
#include <grstaps/timer.hpp>

namespace grstaps
{
    class tabu;
    class Scheduler
    {
       public:
        /**
         *
         * checks if two actions happen at the same time
         *
         * \param index of action
         * \param index of action
         * \return do the timeframes of these actions overlap
         *
         */
        bool checkConcurrent(int first, int second);

        /**
         *
         * initalize schedule
         *
         */
        Scheduler();

        /**
         *
         * copy constructorsched = Scheduler();
         *
         * \param scheduler to copy
         *
         */
        Scheduler(const Scheduler& toCopy);

        /**
         *
         * builds the stn and returns the makespan
         *
         * \param duration of the actions
         * \param ordering constraints
         *
         * \return the makespan of the schedle
         *
         */
        bool schedule(const std::vector<float>& durations, std::vector<std::vector<int>>& orderingConstraints, float longestMP = 0);

        /**
         *
         * returns the makespan of the stn
         *
         * \return the makespan of the stn
         */
        float getMakeSpan();

        /**
         *
         * Adds a constraint to the stn between two actions
         *
         * \param index of action that comes first
         * \param index of action that comes second
         *
         * \return successfully added
         */
        bool addOC(int first, int second);

        /**
         *
         * Adds a constraint from the passed stn between two actions
         *
         * \param index of action that comes first
         * \param index of action that comes second
         * \param the stn that you will be editing
         *
         * \return successfully added
         */
        bool addOCTime(int first,
                       int second,
                       std::vector<std::vector<float>>& stnCopy,
                       std::vector<std::vector<int>>& beforeConstraintVec,
                       std::vector<std::vector<int>>& afterConstraintVec);

        /**
         *
         * Adds a constraint from the passed stn between two actions
         *
         * \param index of action that comes first
         * \param index of action that comes second
         * \param the stn that you will be editing
         *
         * \return successfully added
         */
        float addOCTime2(int first, int second, float newMakespan);

        /**
         *
         * Changes the duration of an action
         *
         * \param index of action
         * \param duration of the action
         *
         * \return successfully changed
         */
        bool increaseActionTime(int actionIndex, float duration);

        /**
         *
         * Changes the duration of an action
         *
         * \param index of action
         * \param duration of the action
         *
         * \return successfully changed
         */
        bool decreaseActionTime(int actionIndex, float duration);

        /**
         *
         * Removes a constraint to the stn between two actions
         *
         * \param index of action that comes first
         * \param index of action that comes second
         *
         */
        void removeOC(int first, int second);

        /**
         *
         * Removes a constraint from the passed stn between two actions
         *
         * \param index of action that comes first
         * \param index of action that comes second
         * \param a stn that you will be editing
         *
         */
        void removeOCTime(int first, int second, std::vector<std::vector<float>>& stnCopy);

        /**
         *
         * Removes a constraint from the passed stn between two actions
         *
         * \param index of action that comes first
         * \param index of action that comes second
         * \param a stn that you will be editing
         *
         */
        float removeOCTime2(int first, int second);

        /**
         *
         * Gets makespan of past stn
         *
         * \param a stn that you wish to find the makespan of
         *
         * \return float denoting the makespan of the stn
         *
         */
        float getMakeSpanSTN(std::vector<std::vector<float>>& stnCopy);

        /**
         *
         * initialize the STN
         *
         * \param the allocation
         * \param duration of the actions
         *
         */
        float initSTN(const std::vector<float>&);

        /**
         *
         * adds an action to the schedule
         *
         * \param the actions duration
         * \param the ordering constraints of the action
         *
         */
        bool addAction(float duration, const std::vector<int>& orderingConstraints);

        /**
         *
         * adds an action to the schedule schedule schedule
         *
         * \param the actions duration
         * \param the ordering constraints of the action
         * \param the disjunctive ordering constraints of the action
         *
         */
        bool addAction(float duration,
                       const std::vector<int>& orderingConstraints,
                       std::vector<std::vector<int>> disorderingConstraints);

        /**
         *
         * remove an action to the schedule
         *
         * \param the index of the action to remove
         *
         */
        bool removeAction(int actionID);

        /**
         *
         * builds the stn and returns the makespan with disjunctive constraints
         *
         * \param duration of the actions
         * \param ordering constraints
         * \param disjunctive constraints
         *
         * \return the makespan of the schedle
         *
         */
        bool schedule(const std::vector<float>&, std::vector<std::vector<int>>&, std::vector<std::vector<int>>&, float = 0);

        /**
         *
         * add disjunctive constraints to the schedule
         *
         * \param constraints to add
         *
         * \return add sucessfully
         *
         */
        void setDisjuctive();

        /**
         *
         * Used as a starting point for the tabu search
         *
         *
         * \return a scheduler with additional ordering constraints for all disjunctive constraints
         *
         */
        void getRandomDisjunct(Scheduler&);

        /**
         *
         * gives makespan of if ordering constraint was added between the two actions
         *
         * \param constraints to add
         *
         * \return adding the ordering constraint will be valid
         *
         */
        bool checkOC(int first, int second);

        /**
         *
         * Prints the schedule
         *
         */
        void printSchedule();

        /**
         *
         * returns the disjunctive ordering constraints
         *
         * \return disjunctive constraints
         *
         */
        int getDisjuctiveSize();

        /**
         *
         * returns the disjunctive ordering as a string
         *
         * \return disjunctive constraints string
         *
         */
        std::string getDisjuctiveID();

        /**
         *
         * Copy the scheduler a d Switch the ith disjunctive ordering
         *
         * \param the number of the disjunctive ordering to switch
         *
         * \return disjunctive constraints string
         *
         */
        bool getShedSwitch(int);

        /**
         *
         * Returns the schedules makespan if the disjunctive ordering was switch for disIndex
         *
         * \param the number of the disjunctive ordering to switch
         *
         * \return makespan after ordering switch
         *
         */
        double getShedSwitchTime(int disIndex);

        /**
         *
         * Updates an stn and a set of constraints passed in adding an ordering constraint between two actions
         *
         * \param index of action that comes first
         * \param index of action that comes second
         * \param the stn that you will be editing
         * \param the list of before constraints
         * \param the list of after constraints
         *
         * \return makespan after ordering switch
         *
         */
        float addOCTemp(int first,
                        int second,
                        std::vector<std::vector<float>> stnCopy,
                        std::vector<std::vector<int>>& beforeConstraintVec,
                        std::vector<std::vector<int>>& afterConstraintVec);

        bool scheduleValid{};                             // is the schedule valid
        std::vector<std::vector<float>> stn;              // stn representing the disjuntive graph
        std::vector<std::vector<int>> beforeConstraints;  // constraints on actions happening before other actions
        std::vector<std::vector<int>> afterConstraints;   // constraints on actions happening after other actions
        float bestSchedule;
        float worstSchedule;
        double makeSpan;
        std::vector<float> actionStartTimes;

       private:
        std::vector<std::vector<int>> disjuctiveConstraints;  // list of disjunctive constraints
        std::vector<int> disjuctiveOrderings;                 // the orderings on those constraints
        int lastAction;
        std::string disID;
        int flag = 1;
        std::vector<std::vector<float>> copySTN;
        std::vector<int> constraintsToUpdate;
        robin_hood::unordered_map<int, std::vector<float>> editedActionTimes;
        float longestMotion;
    };
}  // namespace grstaps
#endif  // GRSTAPS_SCHEDULER_H
