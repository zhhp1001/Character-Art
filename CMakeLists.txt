set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
cmake_minimum_required(VERSION 3.15)
project(opencvTest)

set(CMAKE_CXX_STANDARD 17)
find_package(OpenCV REQUIRED)
include_directories(/usr/local/opencv4)
add_executable(character_art character_art.cpp)
target_link_libraries(character_art ${OpenCV_LIBS})
