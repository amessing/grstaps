#include "icra_problem_v2.hpp"

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
        IcraProblemV2 IcraProblemV2::generate(nlohmann::json& config)
        {
            IcraProblemV2 problem;

            Logger::debug("Generating problem");
            std::random_device rd;
            std::mt19937 gen(rd());

            problem.m_speed_index = 0;
            problem.m_mp_index = 1;

            // 1000m x 1000m
            const float boundary_min = config["mp"]["boundary_min"] = 0.0;
            const float boundary_max = config["mp"]["boundary_max"] = 1000.0;
            config["mp"]["query_time"] = 0.01;
            // Create Locations
            auto obstacles = config["mp"]["obstacles"].get<std::vector<std::vector<b2PolygonShape>>>();
            std::vector<Location> locations;

            // Create locations
            Location hanger = generateLocation(obstacles, locations, gen, boundary_min, boundary_max);

            // Create survivors
            {
                const unsigned int num_survivors = 12;
                Logger::debug("Creating {} survivors", num_survivors);

                // speed, mp, payload
                std::vector<float> traits = {0, 0, 0};
                const std::vector<float> noncum_traits = {0, 0, 0};

                // Half need medicine
                for(unsigned int i = 0; i < num_survivors / 2; ++i)
                {
                    const Location l = generateLocation(obstacles, locations, gen, boundary_min, boundary_max);

                    // Move medicine to survivor
                    problem.m_action_locations->push_back(std::make_pair(0, locations.size() - 1));

                    // Some static time to pick up and drop off
                    problem.m_durations->push_back(350);

                    // Set payload ( 1-2 lbs)
                    traits[2] = std::uniform_real_distribution(1.0, 2.0)(gen);
                    problem.m_goal_distribution->push_back(traits);

                    problem.m_noncum_trait_cutoff->push_back(noncum_traits);
                }

                // Half need food
                for(unsigned int i = 0; i < num_survivors / 2; ++i)
                {
                    const Location l = generateLocation(obstacles, locations, gen, boundary_min, boundary_max);

                    // Move food to survivor
                    problem.m_action_locations->push_back(std::make_pair(0, locations.size() - 1));

                    // Some static time to pick up and drop off
                    problem.m_durations->push_back(300);

                    // Set payload (3-5 lbs)
                    traits[2] = std::uniform_real_distribution(3.0, 4.9)(gen);
                    problem.m_goal_distribution->push_back(traits);

                    problem.m_noncum_trait_cutoff->push_back(noncum_traits);
                }
            }

            config["mp"]["locations"] = locations;
            problem.setupMotionPlanners(config["mp"]);

            // Create robots
            {
                const unsigned int num_robots = problem.m_goal_distribution->size() / 2;
                std::vector<float> traits(3);

                // Half fast but weak (aerial)
                for(unsigned int i = 0; i < num_robots / 2; ++i)
                {
                    traits[0] = 50; // Speed (m/s)
                    traits[1] = 1; // MP
                    traits[2] = 2.5; // Payload (lbs)

                    problem.m_robot_traits->push_back(traits);
                    // Start in the hanger
                    problem.m_starting_locations->push_back(0);
                }

                // Half strong but slow (ground)
                for(unsigned int i = 0; i < num_robots / 2; ++i)
                {
                    traits[0] = 25; // Speed (m/s)
                    traits[1] = 0; // MP
                    traits[2] = 5; // Payload (lbs)

                    problem.m_robot_traits->push_back(traits);
                    // Start in the hanger
                    problem.m_starting_locations->push_back(0);
                }
            }
            return problem;
        }
    }
}