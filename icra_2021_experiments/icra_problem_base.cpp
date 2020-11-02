#include "icra_problem_base.hpp"

// Global
#include <cmath>

// External
#include <boost/make_shared.hpp>

// Grstaps
#include <grstaps/location.hpp>
#include <grstaps/logger.hpp>
#include <grstaps/json_conversions.hpp>
#include <grstaps/motion_planning/motion_planner.hpp>


namespace grstaps
{
    namespace icra2021
    {
        IcraProblemBase::IcraProblemBase()
        {
            m_robot_traits = boost::make_shared<std::vector<std::vector<float>>>();
            m_ordering_constraints = boost::make_shared<std::vector<std::vector<int>>>();
            m_durations = boost::make_shared<std::vector<float>>();
            m_noncum_trait_cutoff = boost::make_shared<std::vector<std::vector<float>>>();
            m_goal_distribution = boost::make_shared<std::vector<std::vector<float>>>();
            m_action_locations = boost::make_shared<std::vector<std::pair<unsigned int, unsigned int>>>();
            m_starting_locations = boost::make_shared<std::vector<unsigned int>>();
            m_motion_planners = boost::make_shared<std::vector<boost::shared_ptr<MotionPlanner>>>();
        }

        void IcraProblemBase::init(const nlohmann::json& data)
        {
            setupMotionPlanners(data["mp"]);

            *m_robot_traits = data["robot_traits"].get<std::vector<std::vector<float>>>();
            *m_ordering_constraints = data["ordering_constraints"].get<std::vector<std::vector<int>>>();
            *m_durations = data["durations"].get<std::vector<float>>();
            *m_noncum_trait_cutoff = data["noncum_trait_cutoff"].get<std::vector<std::vector<float>>>();
            *m_goal_distribution = data["goal_distribution"].get<std::vector<std::vector<float>>>();
            *m_action_locations = data["action_locations"].get<std::vector<std::pair<unsigned int, unsigned int>>>();
            *m_starting_locations = data["starting_locations"].get<std::vector<unsigned int>>();
            m_speed_index = data["speed_index"].get<unsigned int>();
            m_mp_index = data["mp_index"].get<unsigned int>();
        }

        boost::shared_ptr<std::vector<std::vector<int>>>& IcraProblemBase::orderingConstraints()
        {
            return m_ordering_constraints;
        }
        boost::shared_ptr<std::vector<float>>& IcraProblemBase::durations()
        {
            return m_durations;
        }
        boost::shared_ptr<std::vector<std::vector<float>>>& IcraProblemBase::noncumTraitCutoff()
        {
            return m_noncum_trait_cutoff;
        }
        boost::shared_ptr<std::vector<std::vector<float>>>& IcraProblemBase::goalDistribution()
        {
            return m_goal_distribution;
        }
        boost::shared_ptr<std::vector<std::pair<unsigned int, unsigned int>>>& IcraProblemBase::actionLocations()
        {
            return m_action_locations;
        }
        unsigned int IcraProblemBase::speedIndex() const
        {
            return m_speed_index;
        }
        unsigned int IcraProblemBase::mpIndex() const
        {
            return m_mp_index;
        }
        void IcraProblemBase::setupMotionPlanners(const nlohmann::json& config)
        {
            m_mp_json = config;

            auto locations = config["locations"].get<std::vector<Location>>();
            auto obstacles = config["obstacles"].get<std::vector<std::vector<b2PolygonShape>>>();

            //m_motion_planners->clear();
            m_motion_planners->reserve(obstacles.size());

            const float boundary_min      = config["boundary_min"];
            const float boundary_max      = config["boundary_max"];
            const float query_time        = config["query_time"];
            const float connection_range  = config["connection_range"];

            for(int i = 0; i < obstacles.size(); ++i)
            {
                auto motion_planner = boost::make_shared<MotionPlanner>();
                motion_planner->setMap(obstacles[i], boundary_min, boundary_max);
                motion_planner->setLocations(locations);
                motion_planner->setQueryTime(query_time);
                motion_planner->setConnectionRange(connection_range);
                m_motion_planners->push_back(motion_planner);
            }
        }
        boost::shared_ptr<std::vector<boost::shared_ptr<MotionPlanner>>>& IcraProblemBase::motionPlanners()
        {
            return m_motion_planners;
        }
        boost::shared_ptr<std::vector<unsigned int>>& IcraProblemBase::startingLocations()
        {
            return m_starting_locations;
        }
        boost::shared_ptr<std::vector<std::vector<float>>>& IcraProblemBase::robotTraits()
        {
            return m_robot_traits;
        }
        Location IcraProblemBase::generateLocation(const std::string& name,
                                                   const std::vector<std::vector<b2PolygonShape>>& obstacles,
                                                 std::vector<Location>& locations,
                                                 std::mt19937& gen,
                                                 const float boundary_min,
                                                 const float boundary_max)
        {
            b2Transform transform;
            transform.SetIdentity();

            const float boundary_diff = boundary_max - boundary_min;
            const float ep = boundary_diff / 1000;

            while(true)
            {
                b2Vec2 point(std::uniform_real_distribution(boundary_min, boundary_max)(gen),
                             std::uniform_real_distribution(boundary_min, boundary_max)(gen));

                bool success = true;
                for(const std::vector<b2PolygonShape>& set_obstacles: obstacles)
                {
                    for(const b2PolygonShape& obstacle: set_obstacles)
                    {
                        if(obstacle.TestPoint(transform, point))
                        {
                            success = false;
                            break;
                        }
                    }
                    if(!success)
                    {
                        break;
                    }
                }
                if(!success)
                {
                    continue;
                }

                for(const Location& l: locations)
                {
                    if(std::sqrt(pow(l.x() - point.x, 2) + pow(l.y() - point.y, 2)) < ep)
                    {
                        success = false;
                        break;
                    }
                }
                if(!success)
                {
                    continue;
                }

                locations.emplace_back(name, point.x, point.y);
                return locations.back();
            }
        }

        nlohmann::json IcraProblemBase::json() const
        {
            return {
                {"mp", m_mp_json},
                {"robot_traits", *m_robot_traits},
                {"ordering_constraints", *m_ordering_constraints},
                {"durations", *m_durations},
                {"noncum_trait_cutoff", *m_noncum_trait_cutoff},
                {"goal_distribution", *m_goal_distribution},
                {"action_locations", *m_action_locations},
                {"starting_locations", *m_starting_locations},
                {"speed_index", m_speed_index},
                {"mp_index", m_mp_index},
                {"human", m_human}
            };
        }
        void to_json(nlohmann::json& j, const IcraProblemBase& p)
        {
            j = p.json();
        }

        void from_json(const nlohmann::json& j, IcraProblemBase& p)
        {
            p.init(j);
        }
    }
}
