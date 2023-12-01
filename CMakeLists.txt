CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

project(quetzalcoatlus LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${OIDN_DIR}/include)

add_library(lib
    src/image.cpp
    src/random.cpp
    src/render.cpp
    src/shape.cpp
    src/texture.cpp
    src/vec.cpp

    # src/camera.hpp
    # src/image.hpp
    # src/interval.hpp
    # src/material.hpp
    # src/primitive.hpp
    # src/random.hpp
    # src/shape.hpp
    # src/texture.hpp
    # src/vec.hpp

    # src/examples/cornell_box.cpp
)
target_link_libraries(lib ${OpenCV_LIBS} ${OIDN_DIR}/lib/libOpenImageDenoise.so)

add_subdirectory(examples)
