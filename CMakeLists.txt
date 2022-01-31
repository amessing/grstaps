cmake_minimum_required(VERSION 3.10...3.18)
project(grstaps LANGUAGES CXX)
message("Building ${PROJECT_NAME}...")

# Fallback for using newer policies on CMake <3.12.
if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif(${CMAKE_VERSION} VERSION_LESS 3.12)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)

if(NOT DEFINED _opts)
    string(
        APPEND _opts
            #"$<$<CONFIG:DEBUG>:-Og>"
            "$<$<CONFIG:RELEASE>:-O3>"
    )
endif(NOT DEFINED _opts)

# Add external cmake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(FileOutputs)

# Determine if this project is built as a subproject (using add_subdirectory)
# or if it is the master project.
set(MASTER_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(MASTER_PROJECT ON)
    message(STATUS "CMake version: ${CMAKE_VERSION}")
endif (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)

option(BUILD_DOC "Build Documentation" ${MASTER_PROJECT})
option(BUILD_TESTS "Build Unit Tests" ${MASTER_PROJECT})
option(BUILD_EXE "Build Executable" ${MASTER_PROJECT})
option(BUILD_ICRA "Build Experiments for ICRA 2021" OFF)
option(BUILD_IROS "Build Experiments for IROS 2021" ON)
option(BUILD_IJRR "Build Experiments for IJRR 2021" OFF)

# Add external libraries
add_subdirectory(lib)
find_package(OpenMP)

# Collect header and source files
file(GLOB_RECURSE ${PROJECT_NAME}_HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/**.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/**.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/**/**.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/**/**.hpp)
file(GLOB_RECURSE ${PROJECT_NAME}_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/**.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/**/**.cpp)
list(REMOVE_ITEM ${PROJECT_NAME}_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

# Create library
add_library(_${PROJECT_NAME} ${${PROJECT_NAME}_HEADERS} ${${PROJECT_NAME}_SOURCES})
set_target_properties(_${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
target_link_libraries(_${PROJECT_NAME} PRIVATE
        OpenMP::OpenMP_CXX polyclipping earcut box2d fmt nlohmann_json ${OMPL_LIBRARIES} spdlog stdc++fs)
target_include_directories(_${PROJECT_NAME} PUBLIC include PRIVATE ${OMPL_INCLUDE_DIRS})
target_compile_options(_${PROJECT_NAME} PRIVATE ${_opts})

# Create executable
#if(BUILD_EXE)
#    message("Building executable...")
#    add_executable(${PROJECT_NAME} src/main.cpp)
#    set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
#    target_link_libraries(${PROJECT_NAME} PRIVATE
#            box2d nlohmann_json spdlog stdc++fs
#            _${PROJECT_NAME})
#    target_compile_options(${PROJECT_NAME} PRIVATE ${_opts})
#endif(BUILD_EXE)

# Create the experiments for ICRA 2021
if(BUILD_ICRA)
    add_subdirectory(icra_2021_experiments)
endif(BUILD_ICRA)

# Create the experiments for IJRR 2021
if(BUILD_IJRR)
    add_subdirectory(ijrr_2021_experiments)
endif(BUILD_IJRR)

# Create the experiments for IROS 2021
if(BUILD_IROS)
    add_subdirectory(iros_2021_experiments)
endif(BUILD_IROS)

#add_subdirectory(fcpop_mp)

#if(BUILD_DOC)
#    # Check if Doxygen is installed
#    find_package(Doxygen)
#
#    if(DOXYGEN_FOUND)
#        message("Building documentation...")
#
#        # TODO: documentation coverage
#
#        # Set input and output files
#        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
#        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
#
#        # Request to configure the file
#        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
#        message("Doxygen build started")
#
#        add_custom_target(doc ALL
#            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
#            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#            COMMENT "Generating API documentation with Doxygen"
#            VERBATIM)
#    else(DOXYGEN_FOUND)
#        message("Doxygen needs to be installed to generate the doxygen documentation")
#    endif(DOXYGEN_FOUND)
#endif(BUILD_DOC)
#
#if(BUILD_TESTS)
#    message("Building tests...")
#
#    # TODO: coverage?
#
#    file(GLOB_RECURSE ${PROJECT_NAME}_TEST_SOURCES tests/**.[ch]pp)
#    file(GLOB_RECURSE ${PROJECT_NAME}_TEST_DATA
#        tests/data/**
#        tests/data/**/**)
#
#    # Copy test data to outdir/test
#    make_outputs(${CMAKE_CURRENT_SOURCE_DIR} "${${PROJECT_NAME}_TEST_DATA}" ${CMAKE_CURRENT_BINARY_DIR} testDataOutputs)
#
#    add_executable(${PROJECT_NAME}_test ${${PROJECT_NAME}_TEST_SOURCES} ${testDataOutputs})
#    set_target_properties(${PROJECT_NAME}_test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
#    target_link_libraries(${PROJECT_NAME}_test PRIVATE
#            box2d fmt gtest gmock nlohmann_json stdc++fs ${OMPL_LIBRARIES}
#            _${PROJECT_NAME})
#    target_compile_options(${PROJECT_NAME}_test PRIVATE ${_opts})
#    target_include_directories(${PROJECT_NAME}_test PRIVATE ${OMPL_INCLUDE_DIRS})
#endif(BUILD_TESTS)

