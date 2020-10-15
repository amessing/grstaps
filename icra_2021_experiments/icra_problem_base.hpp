#ifndef GRSTAPS_ICRA_PROBLEM_BASE_HPP
#define GRSTAPS_ICRA_PROBLEM_BASE_HPP

// Global
#include <random>
#include <vector>

// External
#include <boost/shared_ptr.hpp>
#include <box2d/box2d.h>
#include <nlohmann/json.hpp>

// Grstaps
#include <grstaps/location.hpp>

namespace grstaps
{
    class MotionPlanner;

    namespace icra2021
    {
        class IcraProblemBase
        {
           public:
            //! \brief Initializes this problem
            void init(const nlohmann::json& data);

            nlohmann::json json() const;

            boost::shared_ptr<std::vector<boost::shared_ptr<MotionPlanner>>>& motionPlanners();
            boost::shared_ptr<std::vector<std::vector<float>>>& robotTraits();
            boost::shared_ptr<std::vector<std::vector<int>>>& orderingConstraints();
            boost::shared_ptr<std::vector<float>>& durations();
            boost::shared_ptr<std::vector<std::vector<float>>>& noncumTraitCutoff();
            boost::shared_ptr<std::vector<std::vector<float>>>& goalDistribution();
            boost::shared_ptr<std::vector<std::pair<unsigned int, unsigned int>>>& actionLocations();
            boost::shared_ptr<std::vector<unsigned int>>& startingLocations();
            unsigned int speedIndex() const;
            unsigned int mpIndex() const;

           protected:
            IcraProblemBase();
            void setupMotionPlanners(const nlohmann::json& data);
            static Location generateLocation(const std::vector<std::vector<b2PolygonShape>>& obstacles,
                                             std::vector<Location>& locations,
                                             std::mt19937& gen,
                                             const float boundary_min,
                                             const float boundary_max);

            // MP
            boost::shared_ptr<std::vector<boost::shared_ptr<MotionPlanner>>> m_motion_planners;

            // TA
            boost::shared_ptr<std::vector<std::vector<float>>> m_robot_traits;
            boost::shared_ptr<std::vector<std::vector<int>>> m_ordering_constraints;
            boost::shared_ptr<std::vector<float>> m_durations;
            boost::shared_ptr<std::vector<std::vector<float>>> m_noncum_trait_cutoff;
            boost::shared_ptr<std::vector<std::vector<float>>> m_goal_distribution;
            boost::shared_ptr<std::vector<std::pair<unsigned int, unsigned int>>> m_action_locations;
            boost::shared_ptr<std::vector<unsigned int>> m_starting_locations;  // for the robots

            unsigned int m_speed_index;
            unsigned int m_mp_index;

            nlohmann::json m_mp_json;
        };

        void to_json(nlohmann::json& j, const IcraProblemBase& p);
        void from_json(const nlohmann::json& j, IcraProblemBase& p);
    }  // namespace icra2021
}  // namespace grstaps


#endif  // GRSTAPS_ICRA_PROBLEM_BASE_HPP
