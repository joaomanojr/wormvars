add_library(flash STATIC flash.c)

# Specify here the include directories exported by library
target_include_directories(flash PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
