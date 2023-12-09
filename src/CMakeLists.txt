target_sources(lib
    PRIVATE
        bxdf.cpp
        camera.cpp
        image.cpp
        random.cpp
        render.cpp
        scene.cpp
        texture.cpp
        transform.cpp
        vec.cpp
)

target_include_directories(lib
    PUBLIC ${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(color)
include_directories(color)
