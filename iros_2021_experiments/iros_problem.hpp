#ifndef GRSTAPS_IROS_PROBLEM_HPP
#define GRSTAPS_IROS_PROBLEM_HPP

// global
#include <memory>
#include <vector>

// external
#include <../lib/unordered_map/robin_hood.h>
#include <box2d/b2_polygon_shape.h>
#include <clipper/clipper.hpp>
#include <nlohmann/json.hpp>
#include <boost/shared_ptr.hpp>

// local
#include "grstaps/location.hpp"

namespace grstaps
{
    namespace iros2021
    {
        class IrosProblem
        {
           public:
            using TraitVector        = std::vector<float>;
            using NonCumVector       = std::vector<float>;
            using RequirementsVector = std::vector<float>;

            IrosProblem();
            void init(const char* parameters, const char* map_file);
            void setLocations(const std::vector<Location>& locations);
            void setActionLocationMap(
                const std::map<std::string, std::pair<unsigned int, unsigned int>>& action_location_map);
            void addActionLocation(const std::string& action, const std::pair<unsigned int, unsigned int>& location);
            void setRobotTraitVector(const std::vector<TraitVector>& robot_traits);
            void setObstacles(const std::vector<std::vector<b2PolygonShape>>& obstacles);
            void setConfig(const nlohmann::json& config);
            void setActionRequirements(const std::vector<std::vector<float>>& actionReq);
            void setActionNonCumRequirements(const std::vector<std::vector<float>>& actionNonCumReq);
            void setStartingLocations(const std::vector<unsigned int>& starting_locations);

            void setWorstMP();

            const std::vector<Location>& locations() const;
            const Location& location(uint i) const;
            const std::pair<unsigned int, unsigned int>& actionLocation(const std::string& name) const;
            std::vector<TraitVector>& robotTraits();
            const TraitVector& robotTrait(uint i) const;
            const std::vector<std::vector<b2PolygonShape>> obstacles() const;
            const std::vector<ClipperLib2::Paths>& map() const;
            const std::vector<unsigned int>& startingLocations() const;
            boost::shared_ptr<std::vector<std::vector<int>>>& orderingConstraints();
            boost::shared_ptr<std::vector<float>>& durations();
            boost::shared_ptr<std::vector<std::vector<float>>>& noncumTraitCutoff();
            boost::shared_ptr<std::vector<std::vector<float>>>& goalDistribution();
            boost::shared_ptr<std::vector<std::pair<unsigned int, unsigned int>>>& taskLocations();
            const nlohmann::json& config() const;

            void writeMap(const std::string& folder);

            robin_hood::unordered_map<std::string, int> actionToRequirements;  //!< Unordered_map to the graphs edges

            std::vector<RequirementsVector> actionRequirements;
            std::vector<NonCumVector> actionNonCumRequirements;
            int speedIndex;
            int mpIndex;
            float longestPath;
            float mp_max;
            float mp_min;



           protected:
            std::vector<b2PolygonShape> convertBuildingsAndStreetsToPolygons1(const nlohmann::json& buildings, const nlohmann::json& streets);
            ClipperLib2::Paths convertBuildingsAndStreetsToPolygons2(const nlohmann::json& buildings, const nlohmann::json& streets);

            std::vector<Location> m_locations;  //!< coordinates and name of location
            std::map<std::string, std::pair<unsigned int, unsigned int>>
                m_action_location_map;                       // mapping of actions to their start and end locations
            std::vector<TraitVector> m_robot_traits;         //!< List of vectors of robot traits
            std::vector<unsigned int> m_starting_locations;  //!< List of the starting location of the robots
            boost::shared_ptr<std::vector<std::vector<int>>> m_ordering_constraints;
            boost::shared_ptr<std::vector<float>> m_durations;
            boost::shared_ptr<std::vector<std::vector<float>>> m_noncum_trait_cutoff;
            boost::shared_ptr<std::vector<std::vector<float>>> m_goal_distribution;
            boost::shared_ptr<std::vector<std::pair<unsigned int, unsigned int>>> m_task_locations;
            std::vector<ClipperLib2::Paths> m_map;
            nlohmann::json m_config;
        };
    }
}

#endif  // GRSTAPS_IROS_PROBLEM_HPP
