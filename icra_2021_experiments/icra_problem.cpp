#include "icra_problem.hpp"

// Global
#include <cmath>
#include <random>

// External
#include <boost/make_shared.hpp>


// Grstaps
#include <grstaps/location.hpp>
#include <grstaps/json_conversions.hpp>
#include <grstaps/motion_planning/motion_planner.hpp>


namespace grstaps
{
    namespace icra2021
    {
        void IcraProblem::init(const nlohmann::json& data)
        {
            setupMotionPlanners(data["mp"]);

            m_robot_traits = data["robot_traits"].get<std::vector<std::vector<float>>>();
            m_ordering_constraints = data["ordering_constraints"].get<std::vector<std::vector<int>>>();
            m_durations = data["durations"].get<std::vector<float>>();
            m_noncum_trait_cutoff = data["noncum_trait_cutoff"].get<std::vector<std::vector<float>>>();
            m_goal_distribution = data["goal_distriution"].get<std::vector<std::vector<float>>>();
            m_action_locations = data["action_locations"].get<std::vector<std::pair<unsigned int, unsigned int>>>();
            m_starting_locations = data["starting_locations"].get<std::vector<unsigned int>>();
            m_speed_index = data["speed_index"].get<unsigned int>();
            m_mp_index = data["mp_index"].get<unsigned int>();
        }

        IcraProblem IcraProblem::generate(const nlohmann::json& config)
        {
            std::random_device rd;
            std::mt19937 gen(rd());

            IcraProblem problem;
            problem.m_speed_index = 0;
            problem.m_mp_index = 5;

            // Create Locations
            auto obstacles = config["mp"]["obstacles"].get<std::vector<std::vector<b2PolygonShape>>>();
            std::vector<Location> locations;


            // Create locations
            Location hanger = generateLocation(obstacles, locations);
            Location hospital = generateLocation(obstacles, locations);
            Location water_tank = generateLocation(obstacles, locations);

            // Create survivors
            {
                const unsigned int num_survivors = std::uniform_int_distribution(3, 10)(gen);

                std::vector<float> traits = {0, 0, 0, 0, 0, 0};
                const std::vector<float> noncum_traits = {0, 0, 0, 0, 0, 0};
                for(unsigned int i = 0; i < num_survivors; ++i)
                {
                    Location l = generateLocation(obstacles, locations);

                    // Move survivor to the hospital
                    problem.m_action_locations.push_back(std::make_pair(locations.size() - 1, 1));

                    // Some static time to pick up and drop off
                    problem.m_durations.push_back(1.0);

                    // Set payload
                    traits[1] = std::uniform_real_distribution(2.0, 3.0)(gen);
                    problem.m_goal_distribution.push_back(traits);

                    problem.m_noncum_trait_cutoff.push_back(noncum_traits);
                }
            }

            // Carry Medicine to Hospital
            {
                const unsigned int num_survivors = problem.m_goal_distribution.size();

                std::vector<float> traits = {0, 0, 0, 0, 0, 0};
                const std::vector<float> noncum_traits = {0, 0, 0, 0, 0, 0};
                for(unsigned int i = 0; i < num_survivors; ++i)
                {
                    // Move medicine to the hospital
                    problem.m_action_locations.push_back(std::make_pair(0, 1));

                    // Some static time to pick up and drop off
                    problem.m_durations.push_back(1.0);

                    // Set payload
                    traits[1] = std::uniform_real_distribution(0.1, 1.0)(gen);
                    problem.m_goal_distribution.push_back(traits);

                    problem.m_noncum_trait_cutoff.push_back(noncum_traits);
                }
            }

            // Heal
            {
                const unsigned int num_survivors = problem.m_goal_distribution.size() / 2;

                std::vector<float> traits = {0, 0, 0, 0, 0, 0};
                const std::vector<float> noncum_traits = {0, 0, 0, 0, 0, 0};
                for(unsigned int i = 0; i < num_survivors; ++i)
                {
                    // No movement
                    problem.m_action_locations.push_back(std::make_pair(1, 1));

                    // Some static time to setup
                    problem.m_durations.push_back(1.0);

                    // Set heal ability
                    traits[3] = std::uniform_real_distribution(1.0, 3.0)(gen);
                    problem.m_goal_distribution.push_back(traits);

                    problem.m_noncum_trait_cutoff.push_back(noncum_traits);
                }

                // Ordering Constraints
                for(unsigned int i = 0; i < num_survivors; ++i)
                {
                    // ordering constraint from moving the survivor to healing
                    problem.m_ordering_constraints.push_back({static_cast<int>(i), static_cast<int>(num_survivors * 2 + i)});

                    // ordering constraint from moving the medicine to healing
                    problem.m_ordering_constraints.push_back({static_cast<int>(num_survivors + i), static_cast<int>(num_survivors * 2 + i)});
                }
            }

            // Put out fire
            {
                const unsigned int num_fires = std::uniform_int_distribution(0, 4)(gen);
                std::vector<float> traits = {0, 0, 0, 0, 0, 0};
                const std::vector<float> noncum_traits = {0, 0, 0, 0, 0, 0};
                for(unsigned int i = 0; i < num_fires; ++i)
                {
                    Location l = generateLocation(obstacles, locations);

                    // Move from the water tank to the fire
                    problem.m_action_locations.push_back(std::pair(2, locations.size() - 1));

                    // Some static time to fill up
                    problem.m_durations.push_back(1.0);

                    // Set water capacity
                    traits[2] = std::uniform_real_distribution(1.0, 3.0)(gen);

                    // Clear Payload and Repair
                    traits[1] = 0;
                    traits[4] = 0;

                    problem.m_goal_distribution.push_back(traits);

                    problem.m_noncum_trait_cutoff.push_back(noncum_traits);

                    // Carry supplies and rebuild
                    if(std::uniform_real_distribution(0.0f, 1.0f)(gen) >= 0.5)
                    {
                        // Carry supplies
                        {
                            // Move from hanger to the fire
                            problem.m_action_locations.push_back(std::pair(0, locations.size() - 1));

                            // Some static time to pick up stuff
                            problem.m_durations.push_back(1.0);

                            // Clear water capacity
                            traits[2] = 0.0;

                            traits[1] = std::uniform_real_distribution(1.0f, 3.0f)(gen);
                            problem.m_goal_distribution.push_back(traits);
                            problem.m_noncum_trait_cutoff.push_back(noncum_traits);

                            problem.m_ordering_constraints.push_back(
                                {static_cast<int>(problem.m_durations.size() - 2),
                                 static_cast<int>(problem.m_durations.size() - 1)});
                        }

                        // Rebuild
                        {
                            // Move to the fire
                            problem.m_action_locations.push_back(std::pair(locations.size() - 1, locations.size() - 1));

                            // Some static time to work
                            problem.m_durations.push_back(10.0);

                            // Clear water capacity and payload
                            traits[1] = 0.0;
                            traits[2] = 0.0;

                            // Set rebuild
                            traits[4] = std::uniform_real_distribution(1.0f, 3.0f)(gen);
                            problem.m_goal_distribution.push_back(traits);
                            problem.m_noncum_trait_cutoff.push_back(noncum_traits);

                            problem.m_ordering_constraints.push_back(
                                {static_cast<int>(problem.m_durations.size() - 2),
                                 static_cast<int>(problem.m_durations.size() - 1)});
                        }

                    }
                }
            }
            config["mp"]["locations"] = locations;
            problem.setupMotionPlanners(config["mp"]);

            // Create Ground Robots
            {
                // speed, payload, water capacity, heal ability, repair ability, mp index
                std::vector<float> traits(6);
                const unsigned int num_ground = std::uniform_int_distribution()(gen);
                for(unsigned int robot_nr = 0; robot_nr < num_ground; ++robot_nr)
                {
                    traits[0] = std::uniform_real_distribution(1.0f, 2.0f)(gen); // Speed
                    traits[1] = std::uniform_real_distribution(3.0f, 4.0f)(gen); // Payload
                    traits[2] = std::uniform_real_distribution(2.0f, 3.0f)(gen); // Water capacity
                    traits[3] = std::uniform_real_distribution(0.0f, 1.0f)(gen); // Heal ability
                    traits[4] = std::uniform_real_distribution(0.0f, 1.0f)(gen); // Repair
                    traits[5] = 0;

                    problem.m_robot_traits.push_back(traits);
                    // Start in the hanger
                    problem.m_starting_locations.push_back(0);
                }
            }

            // Create Aerial Robots
            {
                // speed, payload, water capacity, heal ability, repair ability, mp index
                std::vector<float> traits(6);
                const unsigned int num_aerial = std::uniform_int_distribution()(gen);
                for(unsigned int robot_nr = 0; robot_nr < num_aerial; ++robot_nr)
                {
                    traits[0] = std::uniform_real_distribution(4.0f, 5.0f)(gen); // Speed
                    traits[1] = std::uniform_real_distribution(1.0f, 2.0f)(gen); // Payload
                    traits[2] = std::uniform_real_distribution(1.0f, 2.0f)(gen); // Water capacity
                    traits[3] = std::uniform_real_distribution(0.0f, 1.0f)(gen); // Heal ability
                    traits[4] = std::uniform_real_distribution(0.0f, 1.0f)(gen); // Repair
                    traits[5] = 1;

                    problem.m_robot_traits.push_back(traits);
                    // Start in the hanger
                    problem.m_starting_locations.push_back(0);
                }
            }

            // Create Med Robots
            {
                // speed, payload, water capacity, heal ability, repair ability, mp index
                std::vector<float> traits(6);
                const unsigned int num_med = std::uniform_int_distribution()(gen);
                for(unsigned int robot_nr = 0; robot_nr < num_med; ++robot_nr)
                {
                    traits[0] = std::uniform_real_distribution(0.1f, 1.0f)(gen); // Speed
                    traits[1] = std::uniform_real_distribution(0.0f, 1.0f)(gen); // Payload
                    traits[2] = std::uniform_real_distribution(0.0f, 1.0f)(gen); // Water capacity
                    traits[3] = std::uniform_real_distribution(2.0f, 3.0f)(gen); // Heal ability
                    traits[4] = std::uniform_real_distribution(0.0f, 1.0f)(gen); // Repair
                    traits[5] = 0;

                    problem.m_robot_traits.push_back(traits);
                    // Start in the hospital
                    problem.m_starting_locations.push_back(1);
                }
            }

            // Create Util Robots
            {
                // speed, payload, water capacity, heal ability, repair ability, mp index
                std::vector<float> traits(6);
                const unsigned int num_util = std::uniform_int_distribution()(gen);
                for(unsigned int robot_nr = 0; robot_nr < num_util; ++robot_nr)
                {
                    traits[0] = std::uniform_real_distribution(2.0f, 3.0f)(gen); // Speed
                    traits[1] = std::uniform_real_distribution(2.0f, 3.0f)(gen); // Payload
                    traits[2] = std::uniform_real_distribution(2.0f, 3.0f)(gen); // Water capacity
                    traits[3] = std::uniform_real_distribution(0.0f, 1.0f)(gen); // Heal ability
                    traits[4] = std::uniform_real_distribution(2.0f, 3.0f)(gen); // Repair
                    traits[5] = 0;

                    problem.m_robot_traits.push_back(traits);
                    // Start in the hanger
                    problem.m_starting_locations.push_back(0);
                }
            }

            return problem;
        }
        const std::vector<std::vector<int>>& IcraProblem::orderingConstraints() const
        {
            return m_ordering_constraints;
        }
        const std::vector<float>& IcraProblem::durations() const
        {
            return m_durations;
        }
        const std::vector<std::vector<float>>& IcraProblem::noncumTraitCutoff() const
        {
            return m_noncum_trait_cutoff;
        }
        const std::vector<std::vector<float>>& IcraProblem::goalDistribution() const
        {
            return m_goal_distribution;
        }
        const std::vector<std::pair<unsigned int, unsigned int>>& IcraProblem::actionLocations() const
        {
            return m_action_locations;
        }
        unsigned int IcraProblem::speedIndex() const
        {
            return m_speed_index;
        }
        unsigned int IcraProblem::mpIndex() const
        {
            return m_mp_index;
        }
        void IcraProblem::setupMotionPlanners(const nlohmann::json& config)
        {
            m_mp_json = config;

            auto locations = config["locations"].get<std::vector<Location>>();
            m_starting_locations = config["starting_locations"].get<std::vector<unsigned int>>();
            auto obstacles = config["obstacles"].get<std::vector<std::vector<b2PolygonShape>>>();

            m_motion_planners.clear();
            m_motion_planners.reserve(obstacles.size());

            const float boundary_min      = config["mp_boundary_min"];
            const float boundary_max      = config["mp_boundary_max"];
            const float query_time        = config["mp_query_time"];
            const float connection_range  = config["mp_connection_range"];

            for(int i = 0; i < obstacles.size(); ++i)
            {
                auto motion_planner = boost::make_shared<MotionPlanner>();
                motion_planner->setMap(obstacles[i], boundary_min, boundary_max);
                motion_planner->setLocations(locations);
                motion_planner->setQueryTime(query_time);
                motion_planner->setConnectionRange(connection_range);
                m_motion_planners.push_back(motion_planner);
            }
        }
        std::vector<boost::shared_ptr<MotionPlanner>>& IcraProblem::motionPlanners()
        {
            return m_motion_planners;
        }
        const std::vector<unsigned int>& IcraProblem::startingLocations() const
        {
            return m_starting_locations;
        }
        const std::vector<std::vector<float>>& IcraProblem::robotTraits() const
        {
            return m_robot_traits;
        }
        Location IcraProblem::generateLocation(const std::vector<std::vector<b2PolygonShape>>& obstacles,
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

                return Location("generated", point.x, point.y);
            }
        }

        nlohmann::json IcraProblem::json() const
        {
            return {
                {"mp", m_mp_json},
                {"robot_traits", m_robot_traits},
                {"ordering_constraints", m_ordering_constraints},
                {"durations", m_durations},
                {"noncum_trait_cutoff", m_noncum_trait_cutoff},
                {"goal_distribution", m_goal_distribution},
                {"action_locations", m_action_locations},
                {"starting_locations", m_starting_locations},
                {"speed_index", m_speed_index},
                {"mp_index", m_mp_index}
            };
        }
        void to_json(nlohmann::json& j, const IcraProblem& p)
        {
            j = p.json();
        }

        void from_json(const nlohmann::json& j, IcraProblem& p)
        {
            p.init(j);
        }
    }
}
