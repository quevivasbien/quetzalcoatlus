add_library(color
    rgb.cpp
    spectrum.cpp
    xyz.cpp
)

target_include_directories(color
    PUBLIC ${CMAKE_SOURCE_DIR}/src
)