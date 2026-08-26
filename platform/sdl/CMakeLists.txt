project(sdl)
add_executable(sdl main.cpp)
target_link_libraries(sdl PRIVATE core vendor)
