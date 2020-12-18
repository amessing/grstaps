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
 * You should have received a copy of the GNU Glocation/AllocationExpander.cpp.o
[ 75%] Building CXX object CMakeFiles/_grstaps.dir/src/Search/UniformCostSearch.cpp.o
[ 75%] Building CXX object CMakeFiles/_grstaps.dir/src/Search/SearchBase.cpp.o
[ 76%] Building CXX object CMakeFiles/_grstaps.dir/src/Task_Aeneral Public License
 * along with GRSTAPS; if not, write to the Free Software Foundation,
 * Inc., #59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GRSTAPS_TASKALLOCATION_H
#define GRSTAPS_TASKALLOCATION_H

#include <iomanip>  // std::setw
#include <iostream>
#include <string>
#include <vector>

#include <../lib/unordered_map/robin_hood.h>
#include <grstaps/Connections/taskAllocationToScheduling.h>
#include <grstaps/Task_Allocation/taskAllocationSearchParams.h>


using std::vector;

namespace grstaps
{

    class taskAllocationToScheduling;

    /**
     * Task Allocation
     *
     * \todo Get scheduling working
     * \todo There is a speed memory tade off in distribution
     *
     */
    class TaskAllocation
    {
       public:
        /**
         * default constructor
         *
         */
        TaskAllocation() = default;

        /**
         * constructor
         *
         *\param a vector containing the goaltrait distribution
         *\param a vector containing the species trait distribution
         *\param a vector containing initial allocation
         */
        TaskAllocation(bool,
                       std::shared_ptr<vector<vector<float>>>,
                       vector<vector<float>>*,
                       vector<short>,
                       std::shared_ptr<vector<vector<float>>>,
                       taskAllocationToScheduling,
                       std::shared_ptr<vector<float>>              = nullptr,
                       std::shared_ptr<vector<vector<int>>>        = nullptr,
                       std::shared_ptr<vector<int>>          = nullptr,
                       int                                           = -1,
                       int                                           = -1);

        /**
         * constructoring
         *
         *\param a vector containing the goaltrait distribution
         *\param a vector containing the species trait distribution
         */
        TaskAllocation(bool,
                       std::shared_ptr<vector<vector<float>>>,
                       vector<vector<float>>*,
                       std::shared_ptr<vector<vector<float>>>,
                       taskAllocationToScheduling,
                       std::shared_ptr<vector<float>>              = nullptr,
                       std::shared_ptr<vector<vector<int>>>        = nullptr,
                       std::shared_ptr<vector<int>>                = nullptr,
                       int speedInd                                  = -1,
                       int mpInd                                     = -1);

        /**
         * Copy constructor
         *
         * \return a deep copy of the passed object
         *
         */
        TaskAllocation(TaskAllocation const&);

        /**
         * runs the scheduler on this allocation and returns the time to schedule
         *
         * \param type of schedule you want sufficing or optimal
         *
         * \return bool is this node a goal
         *
         */
        float getScheduleTime();

        /**
         * Is this node a goal node
         *
         * \return bool is this node a goal
         *
         */
        [[nodiscard]] bool isGoalAllocation() const;

        /**
         * does the check to see if the node is a goal
         *
         * \return bool is this node a goal
         *
         */
        [[nodiscard]] bool checkGoalAllocation() const;

        /**
         * fixes the Allocation Trait Distribution
         *
         * \return bool is this node a goal
         *
         */
        void updateAllocationTraitDistribution();

        /**
         * adds an agent to a task
         *
         * \param agent index of agent type to add to task
         * \param task index of task to add agent too
         *
         */
        bool addAgent(int, int);

        /**
         * updates the allocation trait distribution
         *
         * \param agent index of agent type to add to task
         * \param task index of task to add agent too
         *
         */
        void updateAllocationTraitDistributionAgent(int, int);

        /**
         * getter for Allocation
         *
         * \param returns the allocation as a vector<vector<float>>
         *
         */
        [[nodiscard]] const std::vector<short>& getAllocation() const;

        /**
         * getter for getSpeciesTraitDistribution
         *
         * \param returns the getSpeciesTraitDistribution as a vector<vector<float>>
         *
         */
        vector<vector<float>>* getSpeciesTraitDistribution();

        /**
         * getter for Number of each species
         *
         *
         */
        std::shared_ptr<vector<int>> getNumSpecies();

        /**
         * getter for task allocation ID
         *
         *
         */
        std::string getID();

        /**
         * getter for goal distance
         *
         *
         */
        [[nodiscard]] float getGoalDistance() const;

        /**
         * getter for ActionDurations
         *
         *
         */
        std::shared_ptr<vector<float>> getActionDuration();

        /**
         * getter for orderingConstraints
         *
         *
         */
        std::shared_ptr<vector<vector<int>>> getOrderingConstraints();




        /**
         * Adds an action to the task allocators job
         *
         * \param the requirements of the new action
         * \param the cutoff values for all noncumulative traits
         *
         * \return bool is this node a goal
         *
         */
        [[maybe_unused]] void addAction(const vector<float>&, const vector<float>&, float actionDuration, vector<vector<int>>* orderingCon);

        /**
         * getter for  getGoalTraitDistribution
         *
         * \param returns the  getGoalTraitDistribution as a vector<vector<float>>
         *
         */
        [[maybe_unused]] [[nodiscard]] std::shared_ptr<vector<vector<float>>> getGoalTraitDistribution() const;

        /**
         * getter for getAllocationTraitDistribution
         *
         * \param returns the getAllocationTraitDistribution as a vector<vector<float>>
         *
         */
        [[maybe_unused]] [[nodiscard]] const std::vector<std::vector<float>>& getAllocationTraitDistribution() const;

        /**
         * getter for actionNoncumulativeTraitValue
         *
         * \param returns the actionNoncumulativeTraitValue as a vector<vector<float>>
         *
         */
        [[maybe_unused]] std::shared_ptr<vector<vector<float>>> getActionNoncumulativeTraitValue();

        /**
         * setter for Allocation
         *
         *
         */
        [[maybe_unused]] void setAllocation(const std::vector<short>&);

        /**
         * setter for GoalTraitDistribution
         *
         *
         */
        [[maybe_unused]] void setGoalTraitDistribution(std::shared_ptr<vector<vector<float>>>);

        /**
         * setter for peciesTraitDistribution
         *
         *
         */
        [[maybe_unused]] void setSpeciesTraitDistribution(vector<vector<float>>*);

        /**
         * setter for ActionNoncumulativeTraitValue
         *
         *
         */
        [[maybe_unused]] void setActionNoncumulativeTraitValue(std::shared_ptr<vector<vector<float>>>);

        /**
         * setter for Number of each species
         *
         * vector of ints that is the counts
         *
         */
        [[maybe_unused]] void setNumSpecies(std::shared_ptr<vector<int>>);

        /**
         * setter for ActionDuration
         *
         *\param the new action durations
         *
         */
        [[maybe_unused]] void setActionDuration(std::shared_ptr<vector<float>>);

        /**
         * setter for orderingConstraints
         *
         *\param the new orderingconstraints
         *
         */
        [[maybe_unused]] void setOrderingConstraints(std::shared_ptr<vector<vector<int>>>);

        /**
         * getter for numSpecies
         *
         *
         */
        [[maybe_unused]] std::shared_ptr<vector<int>> getnumSpecies();

        /**
         * shows the size of a task allocation object broken down by its parts
         *
         *
         */
        [[maybe_unused]] void checkSize();

        /**
         * Adds an action to the task allocators job
         *
         * \param the new goalTraitDistribution
         * \param the cutoff values for all noncumulative traits
         * \param the new set of action IDS
         * \param the new amount to add to the goal distance
         *
         */
        [[maybe_unused]] void addAction(const vector<float>&, const vector<float>&,
                       float,
                       const float&         = -1,
                       vector<vector<int>>* = nullptr,
                                        bool editParams = false);


        std::shared_ptr<taskAllocationSearchParams> params;
        vector<short> action_dynamics;
        vector<short> allocation;
        vector<vector<float>> requirementsRemaining;
        vector<vector<float>> allocationTraitDistribution;
        taskAllocationToScheduling taToScheduling;

       private:
        float scheduleTime;
        float goalDistance;
        bool isGoal;


    };

}  // namespace grstaps
#endif  // UNTITLED_TASKALLOCATION_H
