target_sources(lib
    PRIVATE
        bxdf.cpp
        camera.cpp
        image.cpp
        material.cpp
        light.cpp
        render.cpp
        sampler.cpp
        scene.cpp
        texture.cpp
        transform.cpp
        util.cpp
        vec.cpp
)

target_include_directories(lib
    PUBLIC ${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(color)
add_subdirectory(obj)
