CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

project(quetzalcoatlus LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(main src/main.cpp)
target_link_libraries(main ${OpenCV_LIBS})