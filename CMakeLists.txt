CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

project(quetzalcoatlus LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${OIDN_DIR}/include)

add_library(lib
    src/random.cpp
    src/render_result.cpp
    src/shape.cpp
    src/texture.cpp
    src/vec.cpp
)
target_link_libraries(lib ${OpenCV_LIBS} ${OIDN_DIR}/lib/libOpenImageDenoise.so)

add_executable(main src/main.cpp)
target_link_libraries(main lib)
