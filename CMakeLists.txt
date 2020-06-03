# Set the minimum version of CMake that can be used
# To find the cmake version run
# $ cmake --version
cmake_minimum_required(VERSION 3.5)

# Set the project name
project (wormvars)

include_directories(include pt-1.4)

add_subdirectory(wormvars)
add_subdirectory(test)