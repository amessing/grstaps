#ifndef GRSTAPS_ICRA_PROBLEM_V1_HPP
#define GRSTAPS_ICRA_PROBLEM_V1_HPP

// Local
#include "icra_problem_base.hpp"

namespace grstaps
{
    class MotionPlanner;

    namespace icra2021
    {
        class IcraProblemV1 : public IcraProblemBase
        {
           public:
            //! \brief Generates a problem for the icra experiments
            static std::unique_ptr<IcraProblemV1> generate(nlohmann::json& config);
        };
    }  // namespace icra2021
}  // namespace grstaps

#endif  // GRSTAPS_ICRA_PROBLEM_V1_HPP
