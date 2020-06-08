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

#include <grstaps/Connections/taskAllocationToScheduling.h>

namespace grstaps
{
    float taskAllocationToScheduling::getNonSpeciesSchedule(vector<short> allocation, vector<float>* actionDurations, vector<vector<int>>* orderingConstraints, boost::shared_ptr<vector<int>> numSpecies){
        std::vector<std::vector<int>> disjunctiveConstraints;
        for(int species=0; species < (*numSpecies).size(); ++species){
            std::vector<int> currentConcurentUse= {};
            int numAction = allocation.size()/(*numSpecies).size();
            for(int action=0; action < numAction; ++action){
                if(allocation[(*numSpecies).size()*action+species] > 0){
                    for(int concur=0; concur < currentConcurentUse.size(); ++concur){
                        vector<int> constraint = {currentConcurentUse[concur],action};
                        auto found = std::find(disjunctiveConstraints.begin(), disjunctiveConstraints.end(), constraint);
                        if(found == disjunctiveConstraints.end()){
                            disjunctiveConstraints.push_back(constraint);
                        }
                    }
                    currentConcurentUse.push_back(action);
                }
            }
        }
        bool valid = sched.schedule(*actionDurations, *orderingConstraints, disjunctiveConstraints);
        if(valid){
            auto sche = sched.getMakeSpan();
            return sched.getMakeSpan();
        }
        return -1;
    }

    float taskAllocationToScheduling::getSpeciesSchedule(vector<short> allocation, vector<float>* actionDurations, vector<vector<int>>* orderingConstraints, boost::shared_ptr<vector<int>> numSpecies)
    {
        throw "Not implemented yet.";
    }
}