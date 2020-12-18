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

            std::shared_ptr<vector<vector<float>>> goalDistribution =
                std::shared_ptr<vector<vector<float>>>(new vector<vector<float>>(vector<vector<float>>{
                    {1.0},
                    {1.0},
                    {1.0},
                }));

            vector<vector<float>> speciesDistribution{
                {1.5},
                {0.5},
                {0.5},
            };

            std::shared_ptr<vector<vector<float>>> noncumTraitCutoff =
                std::shared_ptr<vector<vector<float>>>(new vector<vector<float>>(vector<vector<float>>{
                    {0},{0},{0}

                }));

            std::shared_ptr<vector<int>> numSpec = std::shared_ptr<vector<int>>(new vector<int>{1, 1, 1});
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

            std::shared_ptr<vector<vector<int>>> orderingCon =
                std::shared_ptr<vector<vector<int>>>(new vector<vector<int>>{{0, 2}});

            std::shared_ptr<vector<float>> durations = std::shared_ptr<vector<float>>(new vector<float>(vector<float>{1, 1, 1}));
            taskAllocationToScheduling taToSched = taskAllocationToScheduling();
            bool usingSpecies = false;

            std::shared_ptr<vector<int>> robot_dynamics(new vector<int>(speciesDistribution.size(),0));

            TaskAllocation ta(usingSpecies,
                              goalDistribution,
                              (&speciesDistribution),
                              noncumTraitCutoff,
                              taToSched,
                              durations,
                              orderingCon,
                              numSpec);

            auto node1 = std::shared_ptr<Node<TaskAllocation>>(new Node<TaskAllocation>(std::string(ta.getID()), ta));

            node1->setData(ta);
            Graph<TaskAllocation> graphTest;
            graphTest.addNode(node1);

            std::shared_ptr<Heuristic> heur = std::shared_ptr<Heuristic>(new TAGoalDist());
            std::shared_ptr<Cost> cos       = std::shared_ptr<Cost>(new TAScheduleTime());

            std::shared_ptr<GoalLocator<TaskAllocation>> isGoal =
                std::shared_ptr<GoalLocator<TaskAllocation>>(new AllocationIsGoal());
            std::shared_ptr<AllocationExpander> expander =
                std::shared_ptr<AllocationExpander>(new AllocationExpander(heur, cos));

            SearchResultPackager<TaskAllocation> *package = new AllocationResultsPackager();

            AStarSearch<TaskAllocation> searcher(graphTest, node1);

            AllocationResultsPackager *results = static_cast<AllocationResultsPackager *>(package);
            if(check)
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


            vector<float> newAction{1};
            vector<float> newNonCum{0};
            vector<vector<int>> newOrderingCon;
            string newId = "Action2";
            AStarSearch<TaskAllocation> searcherCopy(searcher, expander, newAction, newNonCum, 1, 1, &newOrderingCon);
            searcherCopy.search(isGoal, expander, package);
            results = static_cast<AllocationResultsPackager *>(package);
            cout << "Copy Node" << endl;
            results->printResults();


            delete package;
        }
    }  // namespace test
}  // namespace grstaps
