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
)
target_link_libraries(lib ${OpenCV_LIBS} ${OIDN_DIR}/lib/libOpenImageDenoise.so)

add_subdirectory(examples)
add_subdirectory(src/color)
