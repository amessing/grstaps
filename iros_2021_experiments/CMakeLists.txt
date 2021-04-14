cmake_minimum_required(VERSION 3.10...3.18)
message("Building experiments for IROS 2021...")

file(GLOB_RECURSE files
        maps/**
        problems/**)

make_outputs(${CMAKE_CURRENT_SOURCE_DIR}/iros_2021_experiments "${files}" ${CMAKE_CURRENT_BINARY_DIR}/iros_2021_experiments configOutputs)

add_executable(experiments main.cpp
        iros_problem.hpp iros_problem.cpp
        iros_solver.hpp iros_solver.cpp
        ${configOutputs})
set_target_properties(experiments PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
target_link_libraries(experiments PRIVATE
        polyclipping args box2d fmt nlohmann_json spdlog stdc++fs
        _${PROJECT_NAME})
target_compile_options(experiments PRIVATE ${_opts})