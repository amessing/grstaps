cmake_minimum_required(VERSION 3.10)
project(grstaps LANGUAGES CXX)
message("Building ${PROJECT_NAME}...")


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(optimization OFF)


# Add external cmake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(FileOutputs)

option(BUILD_DOC "Build Documentation" OFF)
option(BUILD_TESTS "Build Unit Tests" ON)
option(BUILD_EXE "Build Executable" OFF)

# Add external libraries
add_subdirectory(lib)

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
        box2d fmt nlohmann_json ${OMPL_LIBRARIES} spdlog stdc++fs)
target_include_directories(_${PROJECT_NAME} PUBLIC include PRIVATE ${OMPL_INCLUDE_DIRS})

# Create executable
if(BUILD_EXE)
add_executable(${PROJECT_NAME} src/main.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
target_link_libraries(${PROJECT_NAME} PRIVATE
        args box2d nlohmann_json
        _${PROJECT_NAME})
endif(BUILD_EXE)

if(BUILD_DOC)
    # Check if Doxygen is installed
    find_package(Doxygen)

    if(DOXYGEN_FOUND)
        message("Building documentation...")
        
        # TODO: documentation coverage
        
        # Set input and output files
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        # Request to configure the file
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        message("Doxygen build started")

        add_custom_target(doc ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM)
    else(DOXYGEN_FOUND)
        message("Doxygen needs to be installed to generate the doxygen documentation")
    endif(DOXYGEN_FOUND)
endif(BUILD_DOC)

if(BUILD_TESTS)
    message("Building tests...")

    # TODO: coverage?

    file(GLOB_RECURSE ${PROJECT_NAME}_TEST_SOURCES tests/**.[ch]pp)
    file(GLOB_RECURSE ${PROJECT_NAME}_TEST_DATA tests/data/**)

    # Copy test data to outdir/test
    make_outputs(${CMAKE_CURRENT_SOURCE_DIR} "${${PROJECT_NAME}_TEST_DATA}" ${CMAKE_CURRENT_BINARY_DIR} testDataOutputs)

    add_executable(${PROJECT_NAME}_test ${${PROJECT_NAME}_TEST_SOURCES} ${testDataOutputs})
    set_target_properties(${PROJECT_NAME}_test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
    target_link_libraries(${PROJECT_NAME}_test PRIVATE
            box2d fmt gtest gmock nlohmann_json
            _${PROJECT_NAME})
endif(BUILD_TESTS)

set(CMAKE_CXX_FLAGS "-fprofile-arcs -ftest-coverage ")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

IF(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
ENDIF(CMAKE_COMPILER_IS_GNUCC)

IF(optimization)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O3")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O3")
ENDIF(optimization)