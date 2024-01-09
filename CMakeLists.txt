CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

project(quetzalcoatlus LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
# must set OpenImageDenoise_DIR to /path/to/OpenImageDenoise/lib/cmake/OpenImageDenoise-{version}
find_package(OpenImageDenoise 2.1 REQUIRED)
# must set embree_DIR to /path/to/embree/lib/cmake/embree-{version}
find_package(embree 4.3 REQUIRED)

# use boost for regex instead of regex in std lib since it is way faster
find_package(Boost 1.70.0 COMPONENTS regex)

add_library(lib STATIC "")
target_link_libraries(lib
    ${OpenCV_LIBS}
    OpenImageDenoise
    embree
)

if(Boost_FOUND)
    target_link_libraries(lib Boost::regex)
else()
    add_compile_definitions(NO_BOOST)
endif()

add_subdirectory(examples)
add_subdirectory(src)
