#include "icra_problem_v1.hpp"

// Global
#include <cmath>

// Grstaps
#include <grstaps/location.hpp>
#include <grstaps/logger.hpp>
#include <grstaps/json_conversions.hpp>
#include <grstaps/motion_planning/motion_planner.hpp>


namespace grstaps
{
    namespace icra2021
    {
        IcraProblemV1 IcraProblemV1::generate(nlohmann::json& config)
        {
            Logger::debug("Generating problem");
            std::random_device rd;
            std::mt19937 gen(rd());
            const float boundary_min = config["mp"]["boundary_min"];
            const float boundary_max = config["mp"]["boundary_max"];

            IcraProblemV1 problem;
            problem.m_speed_index = 0;
            problem.m_mp_index = 5;

            // Create Locations
            auto obstacles = config["mp"]["obstacles"].get<std::vector<std::vector<b2PolygonShape>>>();
            std::vector<Location> locations;


            // Create locations
            Location hanger = generateLocation(obstacles, locations, gen, boundary_min, boundary_max);
            Location hospital = generateLocation(obstacles, locations, gen, boundary_min, boundary_max);
            Location water_tank = generateLocation(obstacles, locations, gen, boundary_min, boundary_max);

            // Create survivors
            {
                const unsigned int num_survivors = std::uniform_int_distribution(10, 20)(gen);
                Logger::debug("Creating {} survivors", num_survivors);

                std::vector<float> traits = {0, 0, 0, 0, 0, 0};
                const std::vector<float> noncum_traits = {0, 0, 0, 0, 0, 0};
                for(unsigned int i = 0; i < num_survivors; ++i)
                {
                    Location l = generateLocation(obstacles, locations, gen, boundary_min, boundary_max);

                    // Move survivor to the hospital
                    problem.m_action_locations->push_back(std::make_pair(locations.size() - 1, 1));

                    // Some static time to pick up and drop off
                    problem.m_durations->push_back(1.0);

                    // Set payload
                    traits[1] = std::uniform_real_distribution(2.0, 3.0)(gen);
                    problem.m_goal_distribution->push_back(traits);

                    problem.m_noncum_trait_cutoff->push_back(noncum_traits);
                }
            }

            // Carry Medicine to Hospital
            {
                const unsigned int num_survivors = problem.m_goal_distribution->size();

                std::vector<float> traits = {0, 0, 0, 0, 0, 0};
                const std::vector<float> noncum_traits = {0, 0, 0, 0, 0, 0};
                for(unsigned int i = 0; i < num_survivors; ++i)
                {
                    // Move medicine to the hospital
                    problem.m_action_locations->push_back(std::make_pair(0, 1));

                    // Some static time to pick up and drop off
                    problem.m_durations->push_back(1.0);

                    // Set payload
                    traits[1] = std::uniform_real_distribution(0.1, 1.0)(gen);
                    problem.m_goal_distribution->push_back(traits);

                    problem.m_noncum_trait_cutoff->push_back(noncum_traits);
                }
            }

            // Heal
            {
                const unsigned int num_survivors = problem.m_goal_distribution->size() / 2;

                std::vector<float> traits = {0, 0, 0, 0, 0, 0};
                const std::vector<float> noncum_traits = {0, 0, 0, 0, 0, 0};
                for(unsigned int i = 0; i < num_survivors; ++i)
                {
                    // No movement
                    problem.m_action_locations->push_back(std::make_pair(1, 1));

                    // Some static time to setup
                    problem.m_durations->push_back(1.0);

                    // Set heal ability
                    traits[3] = std::uniform_real_distribution(1.0, 3.0)(gen);
                    problem.m_goal_distribution->push_back(traits);

                    problem.m_noncum_trait_cutoff->push_back(noncum_traits);
                }

                // Ordering Constraints
                for(unsigned int i = 0; i < num_survivors; ++i)
                {
                    // ordering constraint from moving the survivor to healing
                    problem.m_ordering_constraints->push_back({static_cast<int>(i), static_cast<int>(num_survivors * 2 + i)});

                    // ordering constraint from moving the medicine to healing
                    problem.m_ordering_constraints->push_back({static_cast<int>(num_survivors + i), static_cast<int>(num_survivors * 2 + i)});
                }
            }

            // Put out fire
            {
                const unsigned int num_fires = std::uniform_int_distribution(5, 10)(gen);
                Logger::debug("Creating {} fires", num_fires);

                std::vector<float> traits = {0, 0, 0, 0, 0, 0};
                const std::vector<float> noncum_traits = {0, 0, 0, 0, 0, 0};
                for(unsigned int i = 0; i < num_fires; ++i)
                {
                    Location l = generateLocation(obstacles, locations, gen, boundary_min, boundary_max);

                    // Move from the water tank to the fire
                    problem.m_action_locations->push_back(std::pair(2, locations.size() - 1));

                    // Some static time to fill up
                    problem.m_durations->push_back(1.0);

                    // Set water capacity
                    traits[2] = std::uniform_real_distribution(1.0, 3.0)(gen);

                    // Clear Payload and Repair
                    traits[1] = 0;
                    traits[4] = 0;

                    problem.m_goal_distribution->push_back(traits);

                    problem.m_noncum_trait_cutoff->push_back(noncum_traits);

                    // Carry supplies and rebuild
                    if(std::uniform_real_distribution(0.0f, 1.0f)(gen) >= 0.5)
                    {
                        // Carry supplies
                        {
                            // Move from hanger to the fire
                            problem.m_action_locations->push_back(std::pair(0, locations.size() - 1));

                            // Some static time to pick up stuff
                            problem.m_durations->push_back(1.0);

                            // Clear water capacity
                            traits[2] = 0.0;

                            traits[1] = std::uniform_real_distribution(1.0f, 3.0f)(gen);
                            problem.m_goal_distribution->push_back(traits);
                            problem.m_noncum_trait_cutoff->push_back(noncum_traits);

                            problem.m_ordering_constraints->push_back(
                                {static_cast<int>(problem.m_durations->size() - 2),
                                 static_cast<int>(problem.m_durations->size() - 1)});
                        }

                        // Rebuild
                        {
                            // Move to the fire
                            problem.m_action_locations->push_back(std::pair(locations.size() - 1, locations.size() - 1));

                            // Some static time to work
                            problem.m_durations->push_back(10.0);

                            // Clear water capacity and payload
                            traits[1] = 0.0;
                            traits[2] = 0.0;

                            // Set rebuild
                            traits[4] = std::uniform_real_distribution(1.0f, 3.0f)(gen);
                            problem.m_goal_distribution->push_back(traits);
                            problem.m_noncum_trait_cutoff->push_back(noncum_traits);

                            problem.m_ordering_constraints->push_back(
                                {static_cast<int>(problem.m_durations->size() - 2),
                                 static_cast<int>(problem.m_durations->size() - 1)});
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
                const unsigned int num_ground = std::uniform_int_distribution(2, 4)(gen);
                Logger::debug("Creating {} ground robots", num_ground);

                for(unsigned int robot_nr = 0; robot_nr < num_ground; ++robot_nr)
                {
                    traits[0] = std::uniform_real_distribution(1.0f, 2.0f)(gen); // Speed
                    traits[1] = std::uniform_real_distribution(3.0f, 4.0f)(gen); // Payload
                    traits[2] = std::uniform_real_distribution(2.0f, 3.0f)(gen); // Water capacity
                    traits[3] = std::uniform_real_distribution(0.0f, 1.0f)(gen); // Heal ability
                    traits[4] = std::uniform_real_distribution(0.0f, 1.0f)(gen); // Repair
                    traits[5] = 0;

                    problem.m_robot_traits->push_back(traits);
                    // Start in the hanger
                    problem.m_starting_locations->push_back(0);
                }
            }

            // Create Aerial Robots
            {
                // speed, payload, water capacity, heal ability, repair ability, mp index
                std::vector<float> traits(6);
                const unsigned int num_aerial = std::uniform_int_distribution(2, 4)(gen);
                Logger::debug("Creating {} aerial robots", num_aerial);

                for(unsigned int robot_nr = 0; robot_nr < num_aerial; ++robot_nr)
                {
                    traits[0] = std::uniform_real_distribution(4.0f, 5.0f)(gen); // Speed
                    traits[1] = std::uniform_real_distribution(1.0f, 2.0f)(gen); // Payload
                    traits[2] = std::uniform_real_distribution(1.0f, 2.0f)(gen); // Water capacity
                    traits[3] = std::uniform_real_distribution(0.0f, 1.0f)(gen); // Heal ability
                    traits[4] = std::uniform_real_distribution(0.0f, 1.0f)(gen); // Repair
                    traits[5] = 1;

                    problem.m_robot_traits->push_back(traits);
                    // Start in the hanger
                    problem.m_starting_locations->push_back(0);
                }
            }

            // Create Med Robots
            {
                // speed, payload, water capacity, heal ability, repair ability, mp index
                std::vector<float> traits(6);
                const unsigned int num_med = std::uniform_int_distribution(2, 4)(gen);
                Logger::debug("Creating {} med robots", num_med);

                for(unsigned int robot_nr = 0; robot_nr < num_med; ++robot_nr)
                {
                    traits[0] = std::uniform_real_distribution(0.1f, 1.0f)(gen); // Speed
                    traits[1] = std::uniform_real_distribution(0.0f, 1.0f)(gen); // Payload
                    traits[2] = std::uniform_real_distribution(0.0f, 1.0f)(gen); // Water capacity
                    traits[3] = std::uniform_real_distribution(2.0f, 3.0f)(gen); // Heal ability
                    traits[4] = std::uniform_real_distribution(0.0f, 1.0f)(gen); // Repair
                    traits[5] = 0;

                    problem.m_robot_traits->push_back(traits);
                    // Start in the hospital
                    problem.m_starting_locations->push_back(1);
                }
            }

            // Create Util Robots
            {
                // speed, payload, water capacity, heal ability, repair ability, mp index
                std::vector<float> traits(6);
                const unsigned int num_util = std::uniform_int_distribution(2, 4)(gen);
                Logger::debug("Creating {} util robots", num_util);

                for(unsigned int robot_nr = 0; robot_nr < num_util; ++robot_nr)
                {
                    traits[0] = std::uniform_real_distribution(2.0f, 3.0f)(gen); // Speed
                    traits[1] = std::uniform_real_distribution(2.0f, 3.0f)(gen); // Payload
                    traits[2] = std::uniform_real_distribution(2.0f, 3.0f)(gen); // Water capacity
                    traits[3] = std::uniform_real_distribution(0.0f, 1.0f)(gen); // Heal ability
                    traits[4] = std::uniform_real_distribution(2.0f, 3.0f)(gen); // Repair
                    traits[5] = 0;

                    problem.m_robot_traits->push_back(traits);
                    // Start in the hanger
                    problem.m_starting_locations->push_back(0);
                }
            }

            return problem;
        }

        /* Normal
        IcraProblem IcraProblem::generate(nlohmann::json& config)
        {
            Logger::debug("Generating problem");
            std::random_device rd;
            std::mt19937 gen(rd());
            const float boundary_min = config["mp"]["boundary_min"];
            const float boundary_max = config["mp"]["boundary_max"];

            IcraProblem problem;
            problem.m_speed_index = 0;
            problem.m_mp_index = 5;

            // Create Locations
            auto obstacles = config["mp"]["obstacles"].get<std::vector<std::vector<b2PolygonShape>>>();
            std::vector<Location> locations;


            // Create locations
            Location hanger = generateLocation(obstacles, locations, gen, boundary_min, boundary_max);
            Location hospital = generateLocation(obstacles, locations, gen, boundary_min, boundary_max);
            Location water_tank = generateLocation(obstacles, locations, gen, boundary_min, boundary_max);

            // Create survivors
            {
                const unsigned int num_survivors = std::uniform_int_distribution(3, 5)(gen);
                Logger::debug("Creating {} survivors", num_survivors);

                std::vector<float> traits = {0, 0, 0, 0, 0, 0};
                const std::vector<float> noncum_traits = {0, 0, 0, 0, 0, 0};
                for(unsigned int i = 0; i < num_survivors; ++i)
                {
                    Location l = generateLocation(obstacles, locations, gen, boundary_min, boundary_max);

                    // Move survivor to the hospital
                    problem.m_action_locations->push_back(std::make_pair(locations.size() - 1, 1));

                    // Some static time to pick up and drop off
                    problem.m_durations->push_back(1.0);

                    // Set payload
                    traits[1] = std::uniform_real_distribution(2.0, 3.0)(gen);
                    problem.m_goal_distribution->push_back(traits);

                    problem.m_noncum_trait_cutoff->push_back(noncum_traits);
                }
            }

            // Carry Medicine to Hospital
            {
                const unsigned int num_survivors = problem.m_goal_distribution->size();

                std::vector<float> traits = {0, 0, 0, 0, 0, 0};
                const std::vector<float> noncum_traits = {0, 0, 0, 0, 0, 0};
                for(unsigned int i = 0; i < num_survivors; ++i)
                {
                    // Move medicine to the hospital
                    problem.m_action_locations->push_back(std::make_pair(0, 1));

                    // Some static time to pick up and drop off
                    problem.m_durations->push_back(1.0);

                    // Set payload
                    traits[1] = std::uniform_real_distribution(0.1, 1.0)(gen);
                    problem.m_goal_distribution->push_back(traits);

                    problem.m_noncum_trait_cutoff->push_back(noncum_traits);
                }
            }

            // Heal
            {
                const unsigned int num_survivors = problem.m_goal_distribution->size() / 2;

                std::vector<float> traits = {0, 0, 0, 0, 0, 0};
                const std::vector<float> noncum_traits = {0, 0, 0, 0, 0, 0};
                for(unsigned int i = 0; i < num_survivors; ++i)
                {
                    // No movement
                    problem.m_action_locations->push_back(std::make_pair(1, 1));

                    // Some static time to setup
                    problem.m_durations->push_back(1.0);

                    // Set heal ability
                    traits[3] = std::uniform_real_distribution(1.0, 3.0)(gen);
                    problem.m_goal_distribution->push_back(traits);

                    problem.m_noncum_trait_cutoff->push_back(noncum_traits);
                }

                // Ordering Constraints
                for(unsigned int i = 0; i < num_survivors; ++i)
                {
                    // ordering constraint from moving the survivor to healing
                    problem.m_ordering_constraints->push_back({static_cast<int>(i), static_cast<int>(num_survivors * 2 + i)});

                    // ordering constraint from moving the medicine to healing
                    problem.m_ordering_constraints->push_back({static_cast<int>(num_survivors + i), static_cast<int>(num_survivors * 2 + i)});
                }
            }

            // Put out fire
            {
                const unsigned int num_fires = std::uniform_int_distribution(0, 3)(gen);
                Logger::debug("Creating {} fires", num_fires);

                std::vector<float> traits = {0, 0, 0, 0, 0, 0};
                const std::vector<float> noncum_traits = {0, 0, 0, 0, 0, 0};
                for(unsigned int i = 0; i < num_fires; ++i)
                {
                    Location l = generateLocation(obstacles, locations, gen, boundary_min, boundary_max);

                    // Move from the water tank to the fire
                    problem.m_action_locations->push_back(std::pair(2, locations.size() - 1));

                    // Some static time to fill up
                    problem.m_durations->push_back(1.0);

                    // Set water capacity
                    traits[2] = std::uniform_real_distribution(1.0, 3.0)(gen);

                    // Clear Payload and Repair
                    traits[1] = 0;
                    traits[4] = 0;

                    problem.m_goal_distribution->push_back(traits);

                    problem.m_noncum_trait_cutoff->push_back(noncum_traits);

                    // Carry supplies and rebuild
                    if(std::uniform_real_distribution(0.0f, 1.0f)(gen) >= 0.5)
                    {
                        // Carry supplies
                        {
                            // Move from hanger to the fire
                            problem.m_action_locations->push_back(std::pair(0, locations.size() - 1));

                            // Some static time to pick up stuff
                            problem.m_durations->push_back(1.0);

                            // Clear water capacity
                            traits[2] = 0.0;

                            traits[1] = std::uniform_real_distribution(1.0f, 3.0f)(gen);
                            problem.m_goal_distribution->push_back(traits);
                            problem.m_noncum_trait_cutoff->push_back(noncum_traits);

                            problem.m_ordering_constraints->push_back(
                                {static_cast<int>(problem.m_durations->size() - 2),
                                 static_cast<int>(problem.m_durations->size() - 1)});
                        }

                        // Rebuild
                        {
                            // Move to the fire
                            problem.m_action_locations->push_back(std::pair(locations.size() - 1, locations.size() - 1));

                            // Some static time to work
                            problem.m_durations->push_back(10.0);

                            // Clear water capacity and payload
                            traits[1] = 0.0;
                            traits[2] = 0.0;

                            // Set rebuild
                            traits[4] = std::uniform_real_distribution(1.0f, 3.0f)(gen);
                            problem.m_goal_distribution->push_back(traits);
                            problem.m_noncum_trait_cutoff->push_back(noncum_traits);

                            problem.m_ordering_constraints->push_back(
                                {static_cast<int>(problem.m_durations->size() - 2),
                                 static_cast<int>(problem.m_durations->size() - 1)});
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
                const unsigned int num_ground = std::uniform_int_distribution(3, 4)(gen);
                Logger::debug("Creating {} ground robots", num_ground);

                for(unsigned int robot_nr = 0; robot_nr < num_ground; ++robot_nr)
                {
                    traits[0] = std::uniform_real_distribution(1.0f, 2.0f)(gen); // Speed
                    traits[1] = std::uniform_real_distribution(3.0f, 4.0f)(gen); // Payload
                    traits[2] = std::uniform_real_distribution(2.0f, 3.0f)(gen); // Water capacity
                    traits[3] = std::uniform_real_distribution(0.0f, 1.0f)(gen); // Heal ability
                    traits[4] = std::uniform_real_distribution(0.0f, 1.0f)(gen); // Repair
                    traits[5] = 0;

                    problem.m_robot_traits->push_back(traits);
                    // Start in the hanger
                    problem.m_starting_locations->push_back(0);
                }
            }

            // Create Aerial Robots
            {
                // speed, payload, water capacity, heal ability, repair ability, mp index
                std::vector<float> traits(6);
                const unsigned int num_aerial = std::uniform_int_distribution(3, 4)(gen);
                Logger::debug("Creating {} aerial robots", num_aerial);

                for(unsigned int robot_nr = 0; robot_nr < num_aerial; ++robot_nr)
                {
                    traits[0] = std::uniform_real_distribution(4.0f, 5.0f)(gen); // Speed
                    traits[1] = std::uniform_real_distribution(1.0f, 2.0f)(gen); // Payload
                    traits[2] = std::uniform_real_distribution(1.0f, 2.0f)(gen); // Water capacity
                    traits[3] = std::uniform_real_distribution(0.0f, 1.0f)(gen); // Heal ability
                    traits[4] = std::uniform_real_distribution(0.0f, 1.0f)(gen); // Repair
                    traits[5] = 1;

                    problem.m_robot_traits->push_back(traits);
                    // Start in the hanger
                    problem.m_starting_locations->push_back(0);
                }
            }

            // Create Med Robots
            {
                // speed, payload, water capacity, heal ability, repair ability, mp index
                std::vector<float> traits(6);
                const unsigned int num_med = std::uniform_int_distribution(3, 4)(gen);
                Logger::debug("Creating {} med robots", num_med);

                for(unsigned int robot_nr = 0; robot_nr < num_med; ++robot_nr)
                {
                    traits[0] = std::uniform_real_distribution(0.1f, 1.0f)(gen); // Speed
                    traits[1] = std::uniform_real_distribution(0.0f, 1.0f)(gen); // Payload
                    traits[2] = std::uniform_real_distribution(0.0f, 1.0f)(gen); // Water capacity
                    traits[3] = std::uniform_real_distribution(2.0f, 3.0f)(gen); // Heal ability
                    traits[4] = std::uniform_real_distribution(0.0f, 1.0f)(gen); // Repair
                    traits[5] = 0;

                    problem.m_robot_traits->push_back(traits);
                    // Start in the hospital
                    problem.m_starting_locations->push_back(1);
                }
            }

            // Create Util Robots
            {
                // speed, payload, water capacity, heal ability, repair ability, mp index
                std::vector<float> traits(6);
                const unsigned int num_util = std::uniform_int_distribution(3, 4)(gen);
                Logger::debug("Creating {} util robots", num_util);

                for(unsigned int robot_nr = 0; robot_nr < num_util; ++robot_nr)
                {
                    traits[0] = std::uniform_real_distribution(2.0f, 3.0f)(gen); // Speed
                    traits[1] = std::uniform_real_distribution(2.0f, 3.0f)(gen); // Payload
                    traits[2] = std::uniform_real_distribution(2.0f, 3.0f)(gen); // Water capacity
                    traits[3] = std::uniform_real_distribution(0.0f, 1.0f)(gen); // Heal ability
                    traits[4] = std::uniform_real_distribution(2.0f, 3.0f)(gen); // Repair
                    traits[5] = 0;

                    problem.m_robot_traits->push_back(traits);
                    // Start in the hanger
                    problem.m_starting_locations->push_back(0);
                }
            }

            return problem;
        }
         */
    }
}
