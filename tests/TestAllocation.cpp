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
 * You should have received a copy of the GNU General Public License
 * along with GRSTAPS; if not, write to the Free Software Foundation,
 * Inc., #59 Temple Plac
 */

// external
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <chrono>
#include <boost/shared_ptr.hpp>
#include <gtest/gtest.h>

// local
#include <grstaps/Task_Allocation/checkAllocatable.h>
#include <grstaps/Graph/Node.h>
#include <grstaps/Graph/Graph.h>
#include <grstaps/Search/UniformCostSearch.h>
#include <grstaps/Task_Allocation/AllocationExpander.h>
#include <grstaps/Task_Allocation/TaskAllocation.h>
#include <grstaps/Task_Allocation/AllocationIsGoal.h>
#include <grstaps/Task_Allocation/AllocationResultsPackager.h>
#include <grstaps/Scheduling/TAScheduleTime.h>
#include <grstaps/Task_Allocation/TAGoalDist.h>
#include <grstaps/Search/AStarSearch.h>
#include <grstaps/Scheduling/Scheduler.h>
#include <grstaps/Search/BestFirstSearch.h>
#include <fstream>



// local

namespace grstaps {
    namespace test {
        TEST(TaskAllocation, test1) {
            bool check = true;
            const std::string id = "Node0";
            float cost = 5.5;

            vector<vector<float>> goalDistribution{
                {0,0, 0, 0,0,0,0,0,1,0},
                {0,0, 0, 0,0,0,0,0,1,0},
                {0,0, 0, 1,0,0,0,0,1,0},
                {0,0, 0, 1,0,0,0,0,1,0},
                {0,0, 0, 0,0,0,0,0,1,0},
                {0,0, 0, 1,0,0,0,0,1,0},
                {0,0, 0, 0,0,0,0,0,1,0},
                {0,0, 0, 0,0,0,0,0,1,0},
                {0,0, 0, 1,0,0,0,0,1,0},
                {0,0, 0, 1,0,0,0,0,1,0},
                {0,0, 0, 0,0,0,0,0,1,0},
                {0,0, 0, 1,0,0,0,0,1,0},
                {0,0, 0, 1,0,0,0,0,1,0},
                {0,0, 0, 0,0,0,0,0,1,0},
                {0,0, 0, 1,0,0,0,0,1,0},
                {0,0, 0, 0,0,0,0,0,1,0},
                {0,0, 0, 1,0,0,0,0,1,0},


            };

            vector<vector<float>> speciesDistribution{
                {0,0, 0, 1,0,0,0,0,0,0},
                {0,0, 0, 0,0,0,0,0,1,0},
                {0,0, 0, 1,0,0,0,0,0,0},
                {0,0, 0, 0,0,0,0,0,1,0},
                {0,0, 0, 1,0,0,0,0,0,0},
                {0,0, 0, 0,0,0,0,0,1,0},
                {0,0, 0, 1,0,0,0,0,0,0},
                {0,0, 0, 0,0,0,0,0,1,0},
                {0,0, 0, 1,0,0,0,0,0,0},
                {0,0, 0, 0,0,0,0,0,1,0},
                {0,0, 0, 1,0,0,0,0,0,0},
                {0,0, 0, 0,0,0,0,0,1,0},

            };

            vector<vector<float>> noncumTraitCutoff{
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0},
                {0,0, 0,0,0,0,0,0,0,0}


            };

            boost::shared_ptr<vector<int>> numSpec = boost::shared_ptr<vector<int>>(new vector<int>{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1});
            vector<float> newCutoff{0,0,0,0,0,0,0,0,0,0};

            vector<vector<float>> actionDist{
                {0,1, 0,0,0.1,0,0,0,0,0},
                {0,1, 0,0,0.1,0,0,0,0,0},
                {0,1, 0,0,0.1,0,0,0,0,0},
                {0,1, 0,0,0.1,0,0,0,0,0},
                {0,1, 0,0,0.1,0,0,0,0,0},
                {0,1, 0,0,0.1,0,0,0,0,0},
                {0,0, 5,0,0.0,0,0,0,0,1},
                {0,0, 5,0,0.0,0,0,0,0,1},
                {0,0, 5,0,0.0,0,0,0,0,1},
                {0,0, 5,0,0.0,0,0,0,0,1},
                {0,0, 5,0,0.0,0,0,0,0,1},
                {0,0, 10,0,0.0,0,0,0,0,1},
                {0,0, 10,0,0.0,0,0,0,0,1},
                {0,0, 0,0,0.0,0,0,1,0,0},
                {0,0, 0,0,0.0,0,0,1,0,0},
                {0,0, 0,0,0.0,0,0,1,0,0},
                {0,1, 0,0,0.0,0,0,1,0,0},
                {0,0, 0,0,0.0,0,0,1,0,0},
                {0,0, 0,0,0.0,0,1,0,0,0},
                {0,0, 0,0,0.0,1,0,0,0,0},
                {0,0, 0,1,0.0,0,0,0,0,0},
                {0,0, 0,10,0.0,0,0,0,0,0},

            };

            vector<vector<int>> orderingCon = vector<vector<int>>{};

            vector<float> durations{10, 10, 10 , 10, 10, 10, 10, 10, 10 , 10, 10, 10, 10, 10, 10, 10, 10};
            taskAllocationToScheduling taToSched;
            bool usingSpecies = false;
            TaskAllocation ta(usingSpecies, (&goalDistribution), (&speciesDistribution), (&noncumTraitCutoff), (&taToSched), (&durations), &orderingCon, numSpec);
            auto node1 = boost::shared_ptr<Node<TaskAllocation>>(new Node<TaskAllocation>(std::string(ta.getID()), ta));

            node1->setData(ta);
            Graph<TaskAllocation> graphTest;
            graphTest.addNode(node1);

            Heuristic *heur = new TAGoalDist();
            Cost *cos = new TAScheduleTime();
            GoalLocator<TaskAllocation> *isGoal = new AllocationIsGoal();
            NodeExpander<TaskAllocation> *expander = new AllocationExpander(heur, cos);
            SearchResultPackager<TaskAllocation> *package = new AllocationResultsPackager();


            AStarSearch<TaskAllocation> searcher(graphTest, node1);


            if(check && isAllocatable(goalDistribution, speciesDistribution, noncumTraitCutoff, numSpec)){
                auto start = std::chrono::high_resolution_clock::now();
                searcher.search(isGoal, expander, package);
                auto stop = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                std::cout << "Time=" << duration.count() << std::endl;
                AllocationResultsPackager *results = static_cast<AllocationResultsPackager *>(package);
                results->printResults();
            }
            else{
                auto start = std::chrono::high_resolution_clock::now();
                searcher.search(isGoal, expander, package);
                auto stop = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                std::cout << "Time=" << duration.count() << std::endl;
                AllocationResultsPackager *results = static_cast<AllocationResultsPackager *>(package);
                results->printResults();
            }

            /*
            vector<float> newAction{0,0, 0,0,0,0,0,0,1,1};
            string newId = "Action2";
            AStarSearch<TaskAllocation> searcherCopy(searcher, newId, newAction, newAction, expander );
            searcherCopy.search(isGoal, expander, package);
            results = static_cast<AllocationResultsPackager *>(package);
            cout << "Copy Node" << endl;
            results->printResults();
            */



            delete isGoal;
            delete expander;
            delete package;
        }
    }
}