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

# Formatting
add_subdirectory(fmt)

# Graph Theory
add_subdirectory(lemon)

# Motion Planning
add_subdirectory(ompl)

# Logging
add_subdirectory(spdlog)
