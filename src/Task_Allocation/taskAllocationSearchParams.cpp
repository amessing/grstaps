//
// Created by glen on 12/6/20.
//

#include "grstaps/Task_Allocation/taskAllocationSearchParams.h"

#include <memory>
#include <utility>


namespace grstaps
{
    taskAllocationSearchParams::taskAllocationSearchParams(
                                                            bool useSpec,
                                                            const std::shared_ptr<vector<vector<float>>>& goalDistribution,
                                                            vector<vector<float>> *speciesDistribution,
                                                            std::shared_ptr<vector<vector<float>>> noncumTraitCutoff,
                                                            std::shared_ptr<vector<float>> actionDur,
                                                            std::shared_ptr<vector<vector<int>>> orderingCon,
                                                            const std::shared_ptr<vector<int>>& numSpec,
                                                            int speedInd,
                                                            int mpIndex){

            usingSpecies = useSpec;
            goalTraitDistribution = goalDistribution;
            speciesTraitDistribution = speciesDistribution;
            actionNoncumulativeTraitValue = std::move(noncumTraitCutoff);
            actionDurations = std::move(actionDur);
            orderingConstraints = std::move(orderingCon);

            if(numSpec != nullptr){
                numSpecies = numSpec;
            }
            else{
                numSpecies = std::make_shared<vector<int>>();
                numSpecies->resize(speciesTraitDistribution->size(), 1);
            }

            speedIndex = speedInd;
            mp_Index = mpIndex;
            std::cout << (*speciesDistribution)[0].size();
            if((*speciesDistribution)[0].size() > 1){
                std::cout << "here";
            }
            traitTeamMax = std::make_shared<vector<float>>((*speciesDistribution)[0].size(), 0);
            maxSpeed = 0;

            startingGoalDistance = 0;


            for(int i = 0; i < numSpec->size(); ++i){
                for(int j = 0; j < (*speciesDistribution)[0].size(); ++j){
                    (*traitTeamMax)[j] += (*speciesDistribution)[i][j] * (*numSpec)[i];
                    if(j == speedInd && maxSpeed < (*speciesDistribution)[i][j]){
                        maxSpeed = (*speciesDistribution)[i][j];
                    }
                }
            }
            }
}

