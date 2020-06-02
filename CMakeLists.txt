# Set the minimum version of CMake that can be used
# To find the cmake version run
# $ cmake --version
cmake_minimum_required(VERSION 3.5)

# Set the project name
project (wormvars)

include_directories(include pt-1.4)

## Add wormvars as a static library
# Declare static library
add_library(wvars STATIC src/wormvars.c)

# Specify here the include directories exported by library
target_include_directories(wvars PUBLIC src)

add_subdirectory(test)