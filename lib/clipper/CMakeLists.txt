cmake_minimum_required(VERSION 3.10)
message("Building clipper")
# define the project
project(polyclipping)

# create and configure the library target
add_library(polyclipping src/clipper.cpp)
set_target_properties(polyclipping PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
target_include_directories(polyclipping PUBLIC include)