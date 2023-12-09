add_executable(cornell_box cornell_box.cpp)
target_link_libraries(cornell_box PRIVATE lib color)
target_include_directories(cornell_box PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(obj_viewer obj_viewer.cpp)
target_link_libraries(obj_viewer PRIVATE lib color)
target_include_directories(obj_viewer PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(opposing_planes opposing_planes.cpp)
target_link_libraries(opposing_planes PRIVATE lib color)
target_include_directories(opposing_planes PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(textures textures.cpp)
target_link_libraries(textures PRIVATE lib color)
target_include_directories(textures PRIVATE ${CMAKE_SOURCE_DIR}/src)
