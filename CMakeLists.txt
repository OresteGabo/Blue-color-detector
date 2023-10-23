cmake_minimum_required(VERSION 3.26)
project(red_detector)

set(CMAKE_CXX_STANDARD 17)

# Find OpenCV package
find_package(OpenCV REQUIRED)

# Add the executable and link with OpenCV libraries
add_executable(red_detector main.cpp)
target_link_libraries(red_detector ${OpenCV_LIBS})

