#include "icra_problem_v1.hpp"

// Global
#include <cmath>

// Grstaps
#include <grstaps/json_conversions.hpp>
#include <grstaps/location.hpp>
#include <grstaps/logger.hpp>
#include <grstaps/motion_planning/motion_planner.hpp>

namespace grstaps::icra2021
{
    std::unique_ptr<IcraProblemV1> IcraProblemV1::generate(nlohmann::json& config)
    {
        std::unique_ptr<IcraProblemV1> problem = std::make_unique<IcraProblemV1>();

        Logger::debug("Generating problem");
        std::random_device rd;
        std::mt19937 gen(rd());

        problem->m_speed_index = 0;
        problem->m_mp_index    = 1;

        // 1000m x 1000m
        const float boundary_min = config["mp"]["boundary_min"] = 0.0;
        const float boundary_max = config["mp"]["boundary_max"] = 3.0;
        config["mp"]["query_time"]                              = 0.01;
        // Create Locations
        auto obstacles = config["mp"]["obstacles"].get<std::vector<std::vector<b2PolygonShape>>>();
        std::vector<Location> locations;

        // Create locations
        Location hanger = generateLocation("hanger", obstacles, locations, gen, boundary_min, boundary_max);
        const unsigned int hanger_index = 0;
        Location hospital = generateLocation("hospital", obstacles, locations, gen, boundary_min, boundary_max);
        const unsigned int hospital_index = 1;

        // Add the config for the problem so a human can know what is going on (num survivors, etc)
        problem->m_human = config;

        // Create survivors
        {
            const unsigned int num_survivors = config["num_survivors"]["total"];
            Logger::debug("Creating {} survivors", num_survivors);

            // speed, mp, payload
            std::vector<float> traits              = {0, 0, 0};
            const std::vector<float> noncumulative_traits = {0, 0, 0};

            const unsigned int num_need_medicine = config["num_survivors"]["need_medicine"];
            // Some need immediate medicine
            for(unsigned int i = 0; i < num_need_medicine; ++i)
            {
                // Survivor location
                const Location l = generateLocation(fmt::format("survivor_med{0}_start", i), obstacles, locations, gen, boundary_min, boundary_max);

                // Move medicine to survivor
                problem->m_action_locations->push_back(std::make_pair(hanger_index, locations.size() - 1));

                // Some static time to pick up and drop off (6 min)
                problem->m_durations->push_back(360);

                // Set payload ( 1-2 lbs)
                traits[2] = std::uniform_real_distribution(1.0, 2.0)(gen);
                problem->m_goal_distribution->push_back(traits);

                problem->m_noncum_trait_cutoff->push_back(noncumulative_traits);
            }

            const unsigned int num_need_food = config["num_survivors"]["need_food"];
            // Some need immediate food
            for(unsigned int i = 0; i < num_need_food; ++i)
            {
                // Survivor location
                const Location l = generateLocation(fmt::format("survivor_food{0}_start", i),obstacles, locations, gen, boundary_min, boundary_max);

                // Move food to survivor
                problem->m_action_locations->push_back(std::make_pair(hanger_index, locations.size() - 1));

                // Some static time to pick up and drop off (5 min)
                problem->m_durations->push_back(300);

                // Set payload (3-5 lbs)
                traits[2] = std::uniform_real_distribution(3.0, 4.9)(gen);
                problem->m_goal_distribution->push_back(traits);

                problem->m_noncum_trait_cutoff->push_back(noncumulative_traits);
            }

            const unsigned int num_need_hospital = config["num_survivors"]["need_hospital"];
            // Some need medical attention
            for(unsigned int i = 0; i < num_need_hospital; ++i)
            {
                // Move survivor to hospital
                {
                    // Survivor location
                    const Location l = generateLocation(fmt::format("survivor_hosp{0}_start", i),obstacles, locations, gen, boundary_min, boundary_max);

                    // Move survivors to hospital
                    problem->m_action_locations->push_back(std::make_pair(locations.size() - 1, hospital_index));

                    // Some static time to pick up and drop off (10 min)
                    problem->m_durations->push_back(600);

                    // Set payload (3-5 lbs)
                    traits[2] = std::uniform_real_distribution(3.0, 4.9)(gen);
                    problem->m_goal_distribution->push_back(traits);

                    problem->m_noncum_trait_cutoff->push_back(noncumulative_traits);
                }

                // Move medicine to hospital
                {
                    problem->m_action_locations->push_back(std::make_pair(0, hospital_index));

                    // Some static time to pick up and drop off (6 min)
                    problem->m_durations->push_back(360);

                    // Set payload ( 1-2 lbs)
                    traits[2] = std::uniform_real_distribution(1.0, 2.0)(gen);
                    problem->m_goal_distribution->push_back(traits);

                    problem->m_noncum_trait_cutoff->push_back(noncumulative_traits);
                }

                // Heal (terminal task)
                {
                    problem->m_action_locations->push_back(std::make_pair(hospital_index, hospital_index));

                    // Instant
                    problem->m_durations->push_back(0);

                    // Set payload 0
                    traits[2] = 0;
                    problem->m_goal_distribution->push_back(traits);

                    problem->m_noncum_trait_cutoff->push_back(noncumulative_traits);
                }

                // Ordering constraints
                {
                    // move survivor to heal
                    problem->m_ordering_constraints->push_back({static_cast<int>(i * 3), static_cast<int>(i * 3 + 2)});
                    // move medicine to heal
                    problem->m_ordering_constraints->push_back({static_cast<int>(i * 3 + 1), static_cast<int>(i * 3 + 2)});
                }
            }
        }

        // Create fires
        {
            const unsigned int num_prior = problem->m_goal_distribution->size();

            // speed, mp, payload
            std::vector<float> traits              = {0, 0, 0};
            const std::vector<float> noncumulative_traits = {0, 0, 0};

            const unsigned int num_fire = config["num_fires"];
            for(unsigned int i = 0; i < num_fire; ++i)
            {
                // Fire location
                const Location l = generateLocation(fmt::format("fire{0}_start", i),obstacles, locations, gen, boundary_min, boundary_max);

                // Bring water
                {
                    problem->m_action_locations->push_back(std::make_pair(hanger_index, locations.size() - 1));

                    // Fill up on water and pray it out
                    problem->m_durations->push_back(600);

                    // Set payload
                    traits[2] = 3.74;
                    problem->m_goal_distribution->push_back(traits);

                    problem->m_noncum_trait_cutoff->push_back(noncumulative_traits);
                }

                // Bring Supplies
                {
                    problem->m_action_locations->push_back(std::make_pair(hanger_index, locations.size() - 1));

                    // Pick up and drop off
                    problem->m_durations->push_back(600);

                    // Set payload
                    traits[2] = 3.74;
                    problem->m_goal_distribution->push_back(traits);

                    problem->m_noncum_trait_cutoff->push_back(noncumulative_traits);
                }

                // Repair
                {
                    problem->m_action_locations->push_back(std::make_pair(locations.size() - 1, locations.size() - 1));

                    // Pick up and drop off
                    problem->m_durations->push_back(3600);

                    // Set payload
                    traits[2] = 1;
                    problem->m_goal_distribution->push_back(traits);

                    problem->m_noncum_trait_cutoff->push_back(noncumulative_traits);
                }

                // Ordering constraints
                {
                    // move water to fire and the bring supplies to repair
                    problem->m_ordering_constraints->push_back({static_cast<int>(num_prior + i * 3), static_cast<int>(num_prior + i * 3 + 1)});
                    // bring supplies to repair before repairing
                    problem->m_ordering_constraints->push_back({static_cast<int>(num_prior + i * 3 + 1), static_cast<int>(num_prior + i * 3 + 2)});
                }
            }
        }

        config["mp"]["locations"] = locations;
        problem->setupMotionPlanners(config["mp"]);

        // Create robots
        {
            const unsigned int num_robots = config["num_robots"]["total"];
            std::vector<float> traits(3);

            // some fast but weak (aerial)
            const unsigned int num_aerial = config["num_robots"]["aerial"];
            for(unsigned int i = 0; i < num_aerial; ++i)
            {
                traits[0] = 1;   // Speed (m/s)
                traits[1] = 1;    // MP
                traits[2] = 2.5;  // Payload (lbs)

                problem->m_robot_traits->push_back(traits);
                // Start in the hanger
                problem->m_starting_locations->push_back(0);
            }

            // some strong but slow (ground)
            const unsigned int num_ground = config["num_robots"]["ground"];
            for(unsigned int i = 0; i < num_ground; ++i)
            {
                traits[0] = 0.5;  // Speed (m/s)
                traits[1] = 0;   // MP
                traits[2] = 5;   // Payload (lbs)

                problem->m_robot_traits->push_back(traits);
                // Start in the hanger
                problem->m_starting_locations->push_back(0);
            }

            // some middle (utility)
            const unsigned int num_utility = config["num_robots"]["utility"];
            for(unsigned int i = 0; i < num_utility; ++i)
            {
                traits[0] = 0.75;  // Speed (m/s)
                traits[1] = 0;   // MP
                traits[2] = 3.75;   // Payload (lbs)

                problem->m_robot_traits->push_back(traits);
                // Start in the hanger
                problem->m_starting_locations->push_back(0);
            }
        }
        return problem;
    }
}  // namespace grstaps::icra2021