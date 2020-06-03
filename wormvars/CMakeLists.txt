add_library(wormvars STATIC wormvars.c)

# Specify here the include directories exported by library
target_include_directories(wormvars PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
