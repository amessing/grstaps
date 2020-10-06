#ifndef GRSTAPS_ICRA_PROBLEM_HPP
#define GRSTAPS_ICRA_PROBLEM_HPP

// Global
#include <vector>

// External
#include <box2d/box2d.h>
#include <boost/shared_ptr.hpp>
#include <nlohmann/json.hpp>

// Grstaps
#include <grstaps/location.hpp>

namespace grstaps
{
    class MotionPlanner;

    namespace icra2021
    {
        class IcraProblem
        {
           public:
            IcraProblem() = default;

            //! \brief Initializes this problem
            void init(const nlohmann::json& data);

            nlohmann::json json() const;
            nlohmann::json mpJson() const;

            //! \brief Generates a problem for the icra experiments
            static IcraProblem generate(const nlohmann::json& config);

            std::vector<boost::shared_ptr<MotionPlanner>>& motionPlanners();
            const std::vector<std::vector<float>>& robotTraits() const;
            const std::vector<std::vector<int>>& orderingConstraints() const;
            const std::vector<float>& durations() const;
            const std::vector<std::vector<float>>& noncumTraitCutoff() const;
            const std::vector<std::vector<float>>& goalDistribution() const;
            const std::vector<std::pair<unsigned int, unsigned int>>& actionLocations() const;
            const std::vector<unsigned int>& startingLocations() const;
            unsigned int speedIndex() const;
            unsigned int mpIndex() const;

           private:
            void setupMotionPlanners(const nlohmann::json& data);
            static Location generateLocation(const std::vector<std::vector<b2PolygonShape>>& obstacles, std::vector<Location>& locations);

            // MP
            std::vector<boost::shared_ptr<MotionPlanner>> m_motion_planners;

            // TA
            std::vector<std::vector<float>> m_robot_traits;
            std::vector<std::vector<int>> m_ordering_constraints;
            std::vector<float> m_durations;
            std::vector<std::vector<float>> m_noncum_trait_cutoff;
            std::vector<std::vector<float>> m_goal_distribution;
            std::vector<std::pair<unsigned int, unsigned int>> m_action_locations;
            std::vector<unsigned int> m_starting_locations; // for the robots

            unsigned int m_speed_index;
            unsigned int m_mp_index;

            nlohmann::json m_mp_json;
        };

        void to_json(nlohmann::json& j, const IcraProblem& p);
        void from_json(const nlohmann::json& j, IcraProblem& p);
    }
}

#endif  // GRSTAPS_ICRA_PROBLEM_HPP
