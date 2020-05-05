//
// Created by glen on 4/5/20.
//

#ifndef UNTITLED_SCHEDULER_H
#define UNTITLED_SCHEDULER_H

#include <map>
#include <vector>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>


class Scheduler {
   public:
    /**
     *
     * builds the stn and returns the makespan
     *
     * \param the allocation
     * \param duration of the actions
     * \param ordering constraints
     *
     * \return the makespan of the schedle
     *
     */
    bool schedule(std::vector<float>*, std::vector<std::vector<int>>*);

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
     * Changes the duration of an action
     *
     * \param index of action
     * \param duration of the action
     *
     * \return successfully changed
     */
    bool increaseActionTime(int actionIndex, int duration);

    /**
     *
     * Changes the duration of an action
     *
     * \param index of action
     * \param duration of the action
     *
     * \return successfully changed
     */
    bool decreaseActionTime(int actionIndex, int duration);

    /**
     *
     * Removes a constraint to the stn between two actions
     *
     * \param index of action that comes first
     * \param index of action that comes second
     *
     * \return successfully added
     */
    bool removeOC(int first, int second);

    /**
     *
     * initialize the STN
     *
     * \param the allocation
     * \param duration of the actions
     *
     */
    float initSTN(std::vector<float>);

    /**
     *
     * adds an action to the schedule
     *
     * \param the actions duration
     * \param the ordering constraints of the action
     *
     */
    bool addAction(float duration, std::vector<int> orderingConstraints);

    /**
    *
    * remove an action to the schedule
    *
    * \param the index of the action to remove
    *
    */
    bool removeAction(int actionID);


   private:
    std::vector<std::vector<float>> stn;
    std::vector<std::vector<int>> beforeConstraints;
    std::vector<std::vector<int>> afterConstraints;
    bool scheduleValid;


};


#endif //UNTITLED_SCHEDULER_H
