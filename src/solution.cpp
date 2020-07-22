//
// Created by glen on 7/20/20.
//

#include <grstaps/solution.hpp>

namespace grstaps
{
    Solution::Solution(std::shared_ptr<Plan> plan, std::shared_ptr<TaskAllocation> allocation){
        m_plan = plan;
        m_allocation = allocation;
    }

}

