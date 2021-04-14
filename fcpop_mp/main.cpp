// Global
#include <signal.h>
#include <fstream>
#include <iostream>
#include <filesystem>

// External
#include <args.hxx>
#include <fmt/format.h>

#include <grstaps/motion_planning/motion_planner.hpp>
#include <grstaps/json_conversions.hpp>

namespace grstaps
{
    namespace fcpop_mp
    {
        void convertToPaths(ClipperLib2::Paths& rv, const std::vector<std::vector<b2Vec2>>& shapes)
        {
            for(const std::vector<b2Vec2>& shape: shapes)
            {
                ClipperLib2::Path polygon;
                for(const b2Vec2& point: shape)
                {
                    polygon.push_back(ClipperLib2::IntPoint(point.x * 1E6, point.y * 1E6));
                }

                // If "hole" flip it
                const float area = ClipperLib2::Area(polygon);
                if(area < 0.0)
                {
                    std::reverse(polygon.begin(), polygon.end());
                }
                rv.push_back(polygon);
            }
        }

        ClipperLib2::Paths convertBuildingsAndStreetsToPolygons2(const nlohmann::json& buildings, const nlohmann::json& streets)
        {
            ClipperLib2::Paths rv;
            convertToPaths(rv, buildings.get<std::vector<std::vector<b2Vec2>>>());
            convertToPaths(rv, streets.get<std::vector<std::vector<b2Vec2>>>());

            // union/smooth out
            {
                ClipperLib2::Clipper clipper;
                clipper.AddPaths(rv, ClipperLib2::ptSubject, true);
                clipper.Execute(ClipperLib2::ctUnion,
                                rv,
                                ClipperLib2::pftNonZero,
                                ClipperLib2::pftNonZero);

                ClipperLib2::ClipperOffset clipper_offset;
                clipper_offset.AddPaths(rv, ClipperLib2::jtMiter, ClipperLib2::etClosedPolygon);
                clipper_offset.Execute(rv, 1E5);
                clipper_offset.Clear();
                clipper_offset.AddPaths(rv, ClipperLib2::jtMiter, ClipperLib2::etClosedPolygon);
                clipper_offset.Execute(rv, -1E5);
            }

            return rv;
        }


        int main(int argc, char** argv)
        {
            args::ArgumentParser parser("MP for FCPOP");
            args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
            args::Positional<std::string> map_file(parser, "map_file", "File containing the map");
            args::Positional<std::string> locations_file(parser, "locations_file", "File containing the locations");
            parser.ParseCLI(argc, argv);

            // Load map json
            std::string map_filepath = map_file.Get();
            std::ifstream ifs(map_filepath);
            nlohmann::json map;
            ifs >> map;

            // Create ground map
            std::vector<ClipperLib2::Paths> map2;
            map2.push_back(convertBuildingsAndStreetsToPolygons2(map["buildings"], map["roads"]));


            // Find boundaries
            float mp_min = std::numeric_limits<float>::max();
            float mp_max = std::numeric_limits<float>::min();
            {

                for(const ClipperLib2::Path& poly: map2[0])
                {
                    for(const ClipperLib2::IntPoint& point: poly)
                    {
                        mp_min = std::min<float>(static_cast<float>(std::min(point.X, point.Y)), mp_min);
                        mp_max = std::max<float>(static_cast<float>(std::max(point.X, point.Y)), mp_max);
                    }
                }
                mp_max += 10;
                mp_min -= 10;
            }

            // Aerial
            ClipperLib2::Path boundary = {
                ClipperLib2::IntPoint(mp_min, mp_min),
                ClipperLib2::IntPoint(mp_min, mp_max),
                ClipperLib2::IntPoint(mp_max, mp_max),
                ClipperLib2::IntPoint(mp_max, mp_min)
            };
            if(ClipperLib2::Area(boundary) < 0)
            {
                std::reverse(boundary.begin(), boundary.end());
            }
            map2.push_back({boundary});

            // Load locations json
            std::string location_filepath = locations_file.Get();
            std::ifstream ifs2(location_filepath);
            nlohmann::json location_info;
            ifs2 >> location_info;

            // Create locations vector
            nlohmann::json location_coords = location_info["coords"];
            std::map<std::string, int> location_indices;
            std::vector<Location> locations;
            for (auto& [key, value] : location_coords.items())
            {
                location_indices[key] = location_indices.size();
                locations.emplace_back(key, value["x"].get<float>(), value["y"].get<float>());
            }

            std::vector<std::array<std::string, 2>> location_pairs = location_info["location_pairs"];

            // Setup MP
            std::vector<MotionPlanner> motion_planners(2);
            for(int i = 0; i < map2.size(); ++i)
            {
                auto& motion_planner = motion_planners[i];
                motion_planner.setMap(map2[i], mp_min, mp_max);
                motion_planner.setLocations(locations);
                motion_planner.setQueryTime(0.1);
                motion_planner.setConnectionRange(1);
            }

            // Create output
            std::vector<std::map<std::string, std::map<std::string, float>>> output(motion_planners.size());
            for(int i = 0; i < motion_planners.size(); ++i)
            {
                std::map<std::string, std::map<std::string, float>>& mp_output = output[i];
                for(const std::array<std::string, 2>& location_pair: location_pairs)
                {
                    std::pair<bool, float> rv = motion_planners[i].query(location_indices[location_pair[0]], location_indices[location_pair[1]]);
                    if(rv.first)
                    {
                        if(mp_output.find(location_pair[0]) == mp_output.end())
                        {
                            mp_output[location_pair[0]] = std::map<std::string, float>();
                        }
                        if(mp_output.find(location_pair[1]) == mp_output.end())
                        {
                            mp_output[location_pair[1]] = std::map<std::string, float>();
                        }

                        mp_output[location_pair[0]][location_pair[1]] = rv.second;
                        mp_output[location_pair[1]][location_pair[0]] = rv.second;
                    }
                }
            }

            nlohmann::json j_output = output;
            std::ofstream ofs;
            ofs.open("mp_output.json");
            ofs << j_output.dump(4);
            ofs.close();

            return 0;
        }
    }
}

int main(int argc, char** argv)
{
    return grstaps::fcpop_mp::main(argc, argv);
}