add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE main.cpp Chip8.cpp Chip8.h)
target_link_libraries(${PROJECT_NAME} PRIVATE vendor)