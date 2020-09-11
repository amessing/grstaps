/*
 * Copyright (C)2020 Andrew Messing
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
 * You should have receive
 * along with GRSTAPS; if not, write to the Free Software Foundation,
 * Inc., #59 Temple Plac
 */

// external
#include <chrono>
#include <iostream>
#include <string>

#include <gtest/gtest.h>

#include <boost/shared_ptr.hpp>

// local
#include <grstaps/Connections/taskAllocationToScheduling.h>
#include <grstaps/Graph/Graph.h>
#include <grstaps/Graph/Node.h>
#include <grstaps/Scheduling/TAScheduleTime.h>
#include <grstaps/Search/AStarSearch.h>
#include <grstaps/Task_Allocation/AllocationExpander.h>
#include <grstaps/Task_Allocation/AllocationIsGoal.h>
#include <grstaps/Task_Allocation/AllocationResultsPackager.h>
#include <grstaps/Task_Allocation/TAGoalDist.h>
#include <grstaps/Task_Allocation/TaskAllocation.h>
#include <grstaps/Task_Allocation/checkAllocatable.h>

// local

namespace grstaps
{
    namespace test
    {
        TEST(TaskAllocation, test1)
        {
            bool check           = true;
            const std::string id = "Node0";
            float cost           = 5.5;

            boost::shared_ptr<vector<vector<float>>> goalDistribution =
                boost::shared_ptr<vector<vector<float>>>(new vector<vector<float>>(vector<vector<float>>{
                    {1.0},
                    {1.0},
                    {1.0},
                }));

            vector<vector<float>> speciesDistribution{
                {1.5},
                {0.5},
                {0.5},
            };

            boost::shared_ptr<vector<vector<float>>> noncumTraitCutoff =
                boost::shared_ptr<vector<vector<float>>>(new vector<vector<float>>(vector<vector<float>>{
                    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

                }));

            boost::shared_ptr<vector<int>> numSpec = boost::shared_ptr<vector<int>>(new vector<int>{1, 1, 1});
            vector<float> newCutoff{0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

            vector<vector<float>> actionDist{
                {0, 1, 0, 0, 0.1, 0, 0, 0, 0, 0},  {0, 1, 0, 0, 0.1, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0.1, 0, 0, 0, 0, 0},
                {0, 1, 0, 0, 0.1, 0, 0, 0, 0, 0},  {0, 1, 0, 0, 0.1, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0.1, 0, 0, 0, 0, 0},
                {0, 0, 5, 0, 0.0, 0, 0, 0, 0, 1},  {0, 0, 5, 0, 0.0, 0, 0, 0, 0, 1}, {0, 0, 5, 0, 0.0, 0, 0, 0, 0, 1},
                {0, 0, 5, 0, 0.0, 0, 0, 0, 0, 1},  {0, 0, 5, 0, 0.0, 0, 0, 0, 0, 1}, {0, 0, 10, 0, 0.0, 0, 0, 0, 0, 1},
                {0, 0, 10, 0, 0.0, 0, 0, 0, 0, 1}, {0, 0, 0, 0, 0.0, 0, 0, 1, 0, 0}, {0, 0, 0, 0, 0.0, 0, 0, 1, 0, 0},
                {0, 0, 0, 0, 0.0, 0, 0, 1, 0, 0},  {0, 1, 0, 0, 0.0, 0, 0, 1, 0, 0}, {0, 0, 0, 0, 0.0, 0, 0, 1, 0, 0},
                {0, 0, 0, 0, 0.0, 0, 1, 0, 0, 0},  {0, 0, 0, 0, 0.0, 1, 0, 0, 0, 0}, {0, 0, 0, 1, 0.0, 0, 0, 0, 0, 0},
                {0, 0, 0, 10, 0.0, 0, 0, 0, 0, 0},

            };

            boost::shared_ptr<vector<vector<int>>> orderingCon =
                boost::shared_ptr<vector<vector<int>>>(new vector<vector<int>>{{0, 2}});

            boost::shared_ptr<vector<float>> durations = boost::shared_ptr<vector<float>>(new vector<float>(vector<float>{1, 1, 1}));
            taskAllocationToScheduling taToSched = taskAllocationToScheduling();
            bool usingSpecies = false;

            boost::shared_ptr<vector<int>> robot_dynamics(new vector<int>(speciesDistribution.size(),0));

            TaskAllocation ta(usingSpecies,
                              goalDistribution,
                              (&speciesDistribution),
                              noncumTraitCutoff,
                              taToSched,
                              durations,
                              orderingCon,
                              numSpec);

            auto node1 = boost::shared_ptr<Node<TaskAllocation>>(new Node<TaskAllocation>(std::string(ta.getID()), ta));

            node1->setData(ta);
            Graph<TaskAllocation> graphTest;
            graphTest.addNode(node1);

            const boost::shared_ptr<Heuristic> heur = boost::shared_ptr<Heuristic>(new TAGoalDist());
            const boost::shared_ptr<Cost> cos       = boost::shared_ptr<Cost>(new TAScheduleTime());

            const boost::shared_ptr<GoalLocator<TaskAllocation>> isGoal =
                boost::shared_ptr<GoalLocator<TaskAllocation>>(new AllocationIsGoal());
            const boost::shared_ptr<AllocationExpander> expander =
                boost::shared_ptr<AllocationExpander>(new AllocationExpander(heur, cos));

            SearchResultPackager<TaskAllocation> *package = new AllocationResultsPackager();

            AStarSearch<TaskAllocation> searcher(graphTest, node1);

            AllocationResultsPackager *results = static_cast<AllocationResultsPackager *>(package);
            if(check && isAllocatable(*goalDistribution, speciesDistribution, *noncumTraitCutoff, numSpec))
            {
                auto start = std::chrono::high_resolution_clock::now();
                searcher.search(isGoal, expander, package);
                auto stop     = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                std::cout << "Time=" << duration.count() << std::endl;
                results->printResults();
            }
            else
            {
                auto start = std::chrono::high_resolution_clock::now();
                searcher.search(isGoal, expander, package);
                auto stop     = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                std::cout << "Time=" << duration.count() << std::endl;
                results->printResults();
            }

            /*
            vector<float> newAction{0,0, 0, 1,0,0,0,0,0,0};
            vector<float> newNonCum{0,0, 0, 1,0,0,0,0,0,0};
            string newId = "Action2";
            AStarSearch<TaskAllocation> searcherCopy(searcher, 10, newAction, newNonCum, expander, &orderingCon);
            searcherCopy.search(isGoal, expander, package);
            results = static_cast<AllocationResultsPackager *>(package);
            cout << "Copy Node" << endl;
            results->printResults();
             */

            delete package;
        }
    }  // namespace test
}  // namespace grstaps
