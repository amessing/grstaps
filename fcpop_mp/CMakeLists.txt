cmake_minimum_required(VERSION 3.10...3.18)
message("Building fcpop mp...")

add_executable(fcpop_mp main.cpp
        ${configOutputs})
set_target_properties(fcpop_mp PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
target_link_libraries(fcpop_mp PRIVATE
        polyclipping args box2d fmt nlohmann_json spdlog stdc++fs
        _${PROJECT_NAME})
target_compile_options(fcpop_mp PRIVATE ${_opts})