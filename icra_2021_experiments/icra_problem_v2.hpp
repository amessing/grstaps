#ifndef GRSTAPS_ICRA_PROBLEM_V2_HPP
#define GRSTAPS_ICRA_PROBLEM_V2_HPP

// Local
#include "icra_problem_base.hpp"

namespace grstaps
{
    class MotionPlanner;

    namespace icra2021
    {
        class IcraProblemV2 : public IcraProblemBase
        {
           public:
            //! \brief Generates a problem for the icra experiments
            static IcraProblemV2 generate(nlohmann::json& config);
        };
    }  // namespace icra2021
}  // namespace grstaps

#endif  // GRSTAPS_ICRA_PROBLEM_V2_HPP
