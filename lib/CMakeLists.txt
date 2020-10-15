cmake_minimum_required(VERSION 3.10...3.18)
message("Building external libraries")

# Testing
if(BUILD_TESTS)
    # Unit tests
    if(NOT TARGET gtest)
        add_subdirectory(googletest)
    endif(NOT TARGET gtest)
endif(BUILD_TESTS)

if(BUILD_EXE)
    if(NOT TARGET args)
        # Argument Parsing
        add_subdirectory(args)
    endif(NOT TARGET args)
endif(BUILD_EXE)

# Collsion Checking
if(NOT TARGET box2d)
    set(BOX2D_BUILD_TESTBED OFF CACHE INTERNAL "")
    set(BOX2D_BUILD_UNIT_TESTS OFF CACHE INTERNAL "")
    set(BOX2D_BUILD_DOCS OFF CACHE INTERNAL "")
    add_subdirectory(box2d)
endif(NOT TARGET box2d)

# Formatting
if(NOT TARGET fmt) # Check if the target is already made
    add_subdirectory(fmt)
endif(NOT TARGET fmt)

# Json (Magic)
if(NOT TARGET nlohmann_json)
    set(JSON_BuildTests OFF CACHE INTERNAL "")
    add_subdirectory(json)
endif(NOT TARGET nlohmann_json)

# Graph Theory
# add_subdirectory(lemon)

# Logging
if(NOT TARGET spdlog)
    add_subdirectory(spdlog)
endif(NOT TARGET spdlog)

# Motion Planning
find_package(ompl 1.4 REQUIRED)
