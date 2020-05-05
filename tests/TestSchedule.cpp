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
#include <grstaps/Scheduling/Scheduler.h>




namespace grstaps {
    namespace test {
        TEST(TaskSchedule, test1) {
            std::vector<float> durations{10,2,5,3};
            std::vector<std::vector<int>> orderingConstraints = std::vector<std::vector<int>>{{0,3},{0,1},{2,1}};
            Scheduler sched;

            std::cout << "Plan Actions" << std::endl;
            for(int i=0; i< durations.size(); ++i){
                std::cout << "Action " << i << ": Duration " << durations[i] << std::endl;
            }
            std::cout << std::endl << "Plan Ordering Constraints" << std::endl;
            for(int i=0; i< orderingConstraints.size(); ++i){
                std::cout << "Action " << orderingConstraints[i][0] << " Before Action " << orderingConstraints[i][1] << std::endl;
            }
            std::cout << std::endl;


            // initialize a schedule
            std::cout << "Creating Schedule" << std::endl;
            std::cout << "Found Schedule is Temporally Valid: " << sched.schedule(&durations, &orderingConstraints) << std::endl;
            std::cout << "Found Schedule Makespan: " << sched.getMakeSpan() << std::endl << std::endl;

            // add an ordering constraint
            std::cout << "Removing Ordering Constraint between Action " << orderingConstraints[0][0] << " and Action " << orderingConstraints[0][1]  << std::endl;
            sched.removeOC(orderingConstraints[0][0],orderingConstraints[0][1]);
            std::cout << "New Schedule Makespan: " << sched.getMakeSpan() << std::endl << std::endl;


            std::cout << "Adding Ordering Constraint between Action " << orderingConstraints[0][0] << " and Action " << orderingConstraints[0][1]  << std::endl;
            sched.addOC(orderingConstraints[0][0],orderingConstraints[0][1]);
            std::cout << "New Schedule Makespan: " << sched.getMakeSpan() << std::endl << std::endl;


            std::cout << "Increasing the Duration of Action " << durations.size() << " From " << durations[durations.size()-1] << " To " << 2 * durations[durations.size()-1] << std::endl;
            sched.increaseActionTime(durations.size()-1, 2 * durations[durations.size()-1]);
            std::cout << "New Schedule Makespan: " << sched.getMakeSpan() << std::endl << std::endl;

            std::cout << "Decreasing the Duration of Action " << durations.size() << " From " << 2 * durations[durations.size()-1] << " To " << durations[durations.size()-1] << std::endl;
            sched.decreaseActionTime(durations.size()-1, durations[durations.size()-1]);
            std::cout << "New Schedule Makespan: " << sched.getMakeSpan() << std::endl << std::endl;

            std::cout << "Removing Action " << 0 << std::endl;
            sched.removeAction(0);
            std::cout << "New Schedule Makespan: " << sched.getMakeSpan() << std::endl << std::endl;


            std::cout << "Adding Action " << 0 << std::endl;
            sched.addAction(10, std::vector<std::vector<int>>{});
            std::cout << "New Schedule Makespan: " << sched.getMakeSpan() << std::endl << std::endl;


        }
    }
}