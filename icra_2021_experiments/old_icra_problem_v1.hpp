#if 0
#ifndef GRSTAPS_OLD_ICRA_PROBLEM_V1_HPP
#define GRSTAPS_OLD_ICRA_PROBLEM_V1_HPP

// Local
#include "icra_problem_base.hpp"

namespace grstaps
{
    class MotionPlanner;

    namespace icra2021
    {
        class OldIcraProblemV1 : public IcraProblemBase
        {
           public:
            //! \brief Generates a problem for the icra experiments
            static std::unique_ptr<OldIcraProblemV1> generate(nlohmann::json& config);
        };
    }  // namespace icra2021
}  // namespace grstaps

#endif  // GRSTAPS_OLD_ICRA_PROBLEM_V1_HPP
#endif