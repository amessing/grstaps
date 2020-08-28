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
#include "grstaps/Task_Allocation/AllocationResultsPackager.h"
#include <fstream>
#include <iostream>

#include "grstaps/Connections/taskAllocationToScheduling.h"

namespace grstaps
{
    void AllocationResultsPackager::printResults()
    {
        if(this->foundGoal)
        {
            std::cout << "Node Found" << std::endl;
            std::cout << "Node= " << this->finalNode->getNodeID() << std::endl;
            std::cout << "Makespan = " << (finalNode->getData().getScheduleTime()) << std::endl;
        }
        else
        {
            std::cout << "Search Failed" << std::endl;
        }
    }

    void AllocationResultsPackager::fileResults(string folder)
    {
        if(this->foundGoal)
        {
            std::ofstream myfile;

            string file = "/TA_Sched.txt";
            file        = folder + file;
            myfile.open(file);

            myfile << "Node Found" << std::endl;
            myfile << "Node= " << this->finalNode->getNodeID() << std::endl;
            myfile << "Makespan = " << (finalNode->getData().getScheduleTime()) << std::endl;

            for(int i = 0; i < finalNode->getData().taToScheduling->sched.stn.size(); ++i)
            {
                myfile << "Action " << i << " start: " << finalNode->getData().taToScheduling->sched.stn[i][0]
                       << " end: " << finalNode->getData().taToScheduling->sched.stn[i][1] << std::endl;
            }

            auto motionPlans =
                finalNode->getData().taToScheduling->saveMotionPlanningNonSpeciesSchedule(&finalNode->getData()).second;
            myfile << endl << "Motion Plans" << endl;
            for(int i = 0; i < motionPlans.size(); ++i)
            {
                myfile << "Agent " << i << "-";
                for(int j = 0; j < motionPlans[i].size(); ++j)
                {
                    myfile << "Motion Plan " << i << endl
                           << "Start: " << motionPlans[i][j].first.first << "End: " << motionPlans[i][j].first.second
                           << endl
                           << "Waypoints" << endl;
                    for(int k = 0; k < motionPlans[i][j].second.size(); ++k)
                    {
                        myfile << "X: " << motionPlans[i][j].second[k].first
                               << "Y: " << motionPlans[i][j].second[k].second;
                    }
                }
                myfile << endl;
            }
        }
        else
        {
            std::cout << "Search Failed" << std::endl;
        }
    }

}  // namespace grstaps
