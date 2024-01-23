add_library(scattering STATIC
    phase_function.cpp
)

target_link_libraries(scattering PRIVATE lib)
