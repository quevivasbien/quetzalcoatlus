CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

project(quetzalcoatlus LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
# must set OpenImageDenoise_DIR to /path/to/OpenImageDenoise/lib/cmake/OpenImageDenoise-{version}
find_package(OpenImageDenoise 2.1 REQUIRED)
# must set embree_DIR to /path/to/embree/lib/cmake/embree-{version}
find_package(embree 4.3 REQUIRED)

add_library(lib
    src/image.cpp
    src/random.cpp
    src/render.cpp
    src/scene.cpp
    src/texture.cpp
    src/vec.cpp
)
target_link_libraries(lib
    ${OpenCV_LIBS}
    OpenImageDenoise
    embree
)

add_subdirectory(examples)

add_executable(hello_embree hello_embree.cpp)
target_link_libraries(hello_embree PRIVATE lib)
target_include_directories(hello_embree PRIVATE ${CMAKE_SOURCE_DIR}/src)