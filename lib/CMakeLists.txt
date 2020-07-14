cmake_minimum_required(VERSION 3.10)
message("Building external libraries")

# Testing
if(BUILD_TESTS)
    # Unit tests
    add_subdirectory(googletest)
endif(BUILD_TESTS)

if(BUILD_EXE)
    # Argument Parsing
    add_subdirectory(args)
endif(BUILD_EXE)

# Collsion Checking
set(BOX2D_BUILD_TESTBED OFF CACHE INTERNAL "")
set(BOX2D_BUILD_UNIT_TESTS OFF CACHE INTERNAL "")
set(BOX2D_BUILD_DOCS OFF CACHE INTERNAL "")
add_subdirectory(box2d)

# Formatting
add_subdirectory(fmt)

# Json (Magic)
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(json)

# Graph Theory
# add_subdirectory(lemon)

# Logging
add_subdirectory(spdlog)

# Motion Planning
find_package(ompl 1.4 REQUIRED)