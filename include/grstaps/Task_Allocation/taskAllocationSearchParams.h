//
// Created by glen on 12/6/20.
//

#ifndef GRSTAPS_TASKALLOCATIONSEARCHPARAMS_H
#define GRSTAPS_TASKALLOCATIONSEARCHPARAMS_H
#include <iomanip>  // std::setw
#include <iostream>
#include <string>
#include <vector>

#include <../lib/unordered_map/robin_hood.h>
#include <boost/shared_ptr.hpp>
#include <grstaps/Connections/taskAllocationToScheduling.h>

namespace grstaps
{
class taskAllocationSearchParams
    {
       public:
            taskAllocationSearchParams(
                    bool useSpec,
                    const std::shared_ptr<vector<vector<float>>>& goalDistribution,
                    vector<vector<float>>* speciesDistribution,
                    std::shared_ptr<vector<vector<float>>> noncumTraitCutoff,
                    std::shared_ptr<vector<float>> actionDur,
                    std::shared_ptr<vector<vector<int>>> orderingCon,
                    const std::shared_ptr<vector<int>>& numSpec,
                    int speedInd,
                    int mpIndex);


            bool usingSpecies;
            std::shared_ptr<vector<vector<float>>> goalTraitDistribution;
            vector<vector<float>>* speciesTraitDistribution;
            std::shared_ptr<vector<vector<float>>> actionNoncumulativeTraitValue{};
            std::shared_ptr<vector<float>> actionDurations;
            std::shared_ptr<vector<vector<int>>> orderingConstraints{};
            std::shared_ptr<vector<int>> numSpecies{};

            int speedIndex;
            int mp_Index;
            float maxSpeed;
            std::shared_ptr<vector<float>> traitTeamMax;

            float startingGoalDistance;

       private:
    };
}


#endif  // GRSTAPS_TASKALLOCATIONSEARCHPARAMS_H
