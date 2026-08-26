add_library(core STATIC src/Chip8.cpp src/Chip8.h)
target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIRECTORY})