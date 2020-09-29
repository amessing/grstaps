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
#include <boost/shared_ptr.hpp>
#include <grstaps/Connections/taskAllocationToScheduling.h>

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
                       const boost::shared_ptr<vector<vector<float>>>,
                       vector<vector<float>>*,
                       vector<short>,
                       boost::shared_ptr<vector<vector<float>>>,
                       taskAllocationToScheduling,
                       boost::shared_ptr<vector<float>>              = nullptr,
                       boost::shared_ptr<vector<vector<int>>>        = nullptr,
                       const boost::shared_ptr<vector<int>>          = nullptr,
                       int                                           = -1,
                       int                                           = -1);

        /**
         * constructoring
         *
         *\param a vector containing the goaltrait distribution
         *\param a vector containing the species trait distribution
         */
        TaskAllocation(bool,
                       const boost::shared_ptr<vector<vector<float>>>,
                       vector<vector<float>>*,
                       boost::shared_ptr<vector<vector<float>>>,
                       taskAllocationToScheduling,
                       boost::shared_ptr<vector<float>>              = nullptr,
                       boost::shared_ptr<vector<vector<int>>>        = nullptr,
                       boost::shared_ptr<vector<int>>                = nullptr,
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
         * Adds an action to the task allocators job
         *
         * \param the requirements of the new action
         * \param the cutoff values for all noncumulative traits
         *
         * \return bool is this node a goal
         *
         */
        void addAction(const vector<float>&,
                       const vector<float>&,
                       const float actionDuration,
                       vector<vector<int>>* orderingCon);

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
        bool isGoalAllocation() const;

        /**
         * does the check to see if the node is a goal
         *
         * \return bool is this node a goal
         *
         */
        bool checkGoalAllocation() const;

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
        const std::vector<short>& getAllocation() const;

        /**
         * getter for  getGoalTraitDistribution
         *
         * \param returns the  getGoalTraitDistribution as a vector<vector<float>>
         *
         */
        boost::shared_ptr<vector<vector<float>>> getGoalTraitDistribution() const;

        /**
         * getter for getAllocationTraitDistribution
         *
         * \param returns the getAllocationTraitDistribution as a vector<vector<float>>
         *
         */
        [[maybe_unused]] const std::vector<std::vector<float>>& getAllocationTraitDistribution() const;

        /**
         * getter for getSpeciesTraitDistribution
         *
         * \param returns the getSpeciesTraitDistribution as a vector<vector<float>>
         *
         */
        vector<vector<float>>* getSpeciesTraitDistribution();

        /**
         * getter for actionNoncumulativeTraitValue
         *
         * \param returns the actionNoncumulativeTraitValue as a vector<vector<float>>
         *
         */
        boost::shared_ptr<vector<vector<float>>> getActionNoncumulativeTraitValue();

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
        [[maybe_unused]] void setGoalTraitDistribution(boost::shared_ptr<vector<vector<float>>>);

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
        [[maybe_unused]] void setActionNoncumulativeTraitValue(boost::shared_ptr<vector<vector<float>>>);

        /**
         * setter for SpeciesNames
         *
         * vector of strings that is the names
         *
         */
        void setSpeciesNames(const std::vector<std::string>&);

        /**
         * setter for ActionNames
         *
         * vector of strings that is the names
         *
         */
        void setActionNames(const std::vector<std::string>&);

        /**
         * setter for Number of each species
         *
         * vector of ints that is the counts
         *
         */
        void setNumSpecies(boost::shared_ptr<vector<int>>);

        /**
         * getter for SpeciesNames
         *
         *
         */
        robin_hood::unordered_map<std::string, int>* getSpeciesNames();

        /**
         * getter for ActionDurations
         *
         *
         */
        boost::shared_ptr<vector<float>> getActionDuration();

        /**
         * setter for ActionDuration
         *
         *\param the new action durations
         *
         */
        void setActionDuration(boost::shared_ptr<vector<float>>);

        /**
         * getter for orderingConstraints
         *
         *
         */
        boost::shared_ptr<vector<vector<int>>> getOrderingConstraints();

        /**
         * setter for orderingConstraints
         *
         *\param the new orderingconstraints
         *
         */
        void setOrderingConstraints(boost::shared_ptr<vector<vector<int>>>);

        /**
         * getter for Number of each species
         *
         *
         */
        boost::shared_ptr<vector<int>> getNumSpecies();

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
        float getGoalDistance() const;

        /**
         * getter for numSpecies
         *
         *
         */
        [[maybe_unused]] boost::shared_ptr<vector<int>> getnumSpecies();

        /**
         * shows the size of a task allocation object broken down by its parts
         *
         *
         */
        void checkSize();

        /**
         * Adds an action to the task allocators job
         *
         * \param the new goalTraitDistribution
         * \param the cutoff values for all noncumulative traits
         * \param the new set of action IDS
         * \param the new amount to add to the goal distance
         *
         */
        void addAction(const vector<float>&,
                       const vector<float>&,
                       float,
                       const float&         = -1,
                       vector<vector<int>>* = NULL);

        vector<short> allocation;
        boost::shared_ptr<vector<float>> traitTeamMax;
        vector<vector<float>> requirementsRemaining;
        vector<vector<float>> allocationTraitDistribution;
        boost::shared_ptr<vector<vector<float>>> goalTraitDistribution;
        float* startingGoalDistance;
        taskAllocationToScheduling taToScheduling;
        boost::shared_ptr<vector<float>> actionDurations;
        int speedIndex;
        float maxSpeed;
        int mp_Index;
        vector<vector<float>>* speciesTraitDistribution;
        vector<int> action_dynamics;

       private:

        boost::shared_ptr<vector<int>> numSpecies{};
        boost::shared_ptr<vector<vector<float>>> actionNoncumulativeTraitValue{};
        boost::shared_ptr<vector<vector<int>>> orderingConstraints{};
        float scheduleTime;
        float goalDistance;


        bool isGoal;
        bool usingSpecies;

    };

}  // namespace grstaps
#endif  // UNTITLED_TASKALLOCATION_H
