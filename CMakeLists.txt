CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

project(quetzalcoatlus LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
# must set OpenImageDenoise_DIR to /path/to/OpenImageDenoise/lib/cmake/OpenImageDenoise-{version}
find_package(OpenImageDenoise 2.1 REQUIRED)
# must set embree_DIR to /path/to/embree/lib/cmake/embree-{version}
find_package(embree 4.3 REQUIRED)

add_library(lib STATIC "")
target_link_libraries(lib
    ${OpenCV_LIBS}
    OpenImageDenoise
    embree
)

add_subdirectory(examples)
add_subdirectory(src)
