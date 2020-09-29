//
// Created by glen on 4/30/20.
//

#include "grstaps/Task_Allocation/checkAllocatable.h"

namespace grstaps
{
    bool isAllocatable(const std::vector<std::vector<float>>& goalDistribution,
                       const std::vector<std::vector<float>>& speciesDistribution,
                       const std::vector<std::vector<float>>& nonCumTraitCutoff,
                       boost::shared_ptr<std::vector<int>> numSpec)
    {
        std::vector<float> traitMax;
        for(int j = 0; j < speciesDistribution[0].size(); ++j)
        {
            traitMax.push_back(0.0);
            for(int i = 0; i < speciesDistribution.size(); ++i)
            {
                traitMax[j] += speciesDistribution[i][j] * (*numSpec)[i];
            }
        }
        for(int k = 0; k < goalDistribution.size(); ++k)
        {
            for(int i = 0; i < speciesDistribution[0].size(); ++i)
            {
                if(nonCumTraitCutoff[k][i] == 0)
                {
                    if(goalDistribution[k][i] > traitMax[i])
                    {
                        return false;
                    }
                }
                else
                {
                    int count = 0;
                    for(int t = 0; t < speciesDistribution.size(); ++t)
                    {
                        if(speciesDistribution[t][i] >= nonCumTraitCutoff[k][i])
                        {
                            count += (*numSpec)[i];
                        }
                    }
                    if(count < goalDistribution[k][i])
                    {
                        return false;
                    }
                }
            }
        }
        return true;
    }
}