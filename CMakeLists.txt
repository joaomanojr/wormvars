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


#### From https://github.com/kaizouman/gtest-cmake-example/blob/master/test/CMakeLists.txt

find_package(Threads REQUIRED)

# Enable ExternalProject CMake module
include(ExternalProject)

# Download and install GoogleTest
ExternalProject_Add(
    gtest
    URL https://github.com/google/googletest/archive/master.zip
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
    # Disable install step
    INSTALL_COMMAND ""
)

# Get GTest source and binary directories from CMake project
ExternalProject_Get_Property(gtest source_dir binary_dir)

# Create a libgtest target to be used as a dependency by test programs
add_library(libgtest IMPORTED STATIC GLOBAL)
add_dependencies(libgtest gtest)

# Set libgtest properties
set_target_properties(libgtest PROPERTIES
    "IMPORTED_LOCATION" "${binary_dir}/lib/libgtest.a"
    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
)

# Create a libgmock target to be used as a dependency by test programs
add_library(libgmock IMPORTED STATIC GLOBAL)
add_dependencies(libgmock gtest)

# -- # Set libgmock properties
# -- set_target_properties(libgmock PROPERTIES
# --     "IMPORTED_LOCATION" "${binary_dir}/googlemock/libgmock.a"
# --     "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
# -- )

# I couldn't make it work with INTERFACE_INCLUDE_DIRECTORIES
include_directories("${source_dir}/googletest/include"
                    "${source_dir}/googlemock/include")

#### end GTEST available


# Add test executable
add_executable(test_wvars test/main.cc test/test.cc)

# -- TARGET_LINK_LIBRARIES(test_wvars libwvars libgtest libgmock)
TARGET_LINK_LIBRARIES(test_wvars wvars libgtest)