target_sources(lib
    PRIVATE
        camera.cpp
        image.cpp
        random.cpp
        render.cpp
        scene.cpp
        texture.cpp
        vec.cpp
)

target_include_directories(lib
    PUBLIC ${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(color)