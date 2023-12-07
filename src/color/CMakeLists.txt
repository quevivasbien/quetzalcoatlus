add_library(color STATIC
    rgb.cpp
    rgb_to_spectrum_opt.cpp
    sensor.cpp
    spectrum.cpp
    spectrum_sample.cpp
    xyz.cpp
)

target_link_libraries(color PRIVATE lib)

add_executable(color_test
    color_test.cpp)

target_link_libraries(color_test PRIVATE color lib)
