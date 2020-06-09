add_library(llmock STATIC flash.cc FlashMock.cc crc32.c timer.c)

# Specify here the include directories exported by library
target_include_directories(llmock PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
