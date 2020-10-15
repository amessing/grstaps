cmake_minimum_required(VERSION 3.10...3.18)
message("Building experiments for ICRA 2021...")

file(GLOB_RECURSE icra_2021_files
        maps/**
        problems/**)
make_outputs(${CMAKE_CURRENT_SOURCE_DIR}/icra_2021_experiments "${icra_2021_files}" ${CMAKE_CURRENT_BINARY_DIR}/icra_2021_experiments configOutputs)

add_executable(icra_experiments main.cpp
        icra_problem_base.hpp icra_problem_base.cpp
        icra_problem_v1.hpp icra_problem_v1.cpp
        icra_problem_v2.hpp icra_problem_v2.cpp
        experiments.hpp experiments.cpp
        ${configOutputs})
set_target_properties(icra_experiments PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
target_link_libraries(icra_experiments PRIVATE
        args box2d fmt nlohmann_json spdlog stdc++fs
        _${PROJECT_NAME})
target_compile_options(icra_experiments PRIVATE ${_opts})