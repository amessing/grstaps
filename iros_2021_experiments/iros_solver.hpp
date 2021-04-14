#ifndef GRSTAPS_IROS_SOLVER_HPP
#define GRSTAPS_IROS_SOLVER_HPP

#include <vector>

#include "iros_problem.hpp"
#include <boost/shared_ptr.hpp>
#include <grstaps/Task_Allocation/TaskAllocation.h>
#include <grstaps/motion_planning/motion_planner.hpp>

namespace grstaps
{
    namespace iros2021
    {
        class IrosSolver
        {
           public:
                nlohmann::json solve(IrosProblem& problem);
           private:
                boost::shared_ptr<std::vector<boost::shared_ptr<MotionPlanner>>> setupMotionPlanners(const IrosProblem& problem);
                nlohmann::json solutionToJson(TaskAllocation& allocation);
        };
    }
}

#endif  // GRSTAPS_IROS_SOLVER_HPP
