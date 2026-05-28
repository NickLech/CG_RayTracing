add_library(Geometry STATIC
    "src/geometry.cpp"
    "src/cube.cpp"
    "src/triangle.cpp"
    "src/sphere.cpp"
    "src/hit_record.cpp"
    "src/kd_tree.cpp"
    "src/hittable.cpp"
    "src/material.cpp"
    "src/mesh.cpp"
    "./src/texture.cpp"
    "include/geometry.hpp"
    "include/cube.hpp"
    "include/triangle.hpp"
    "include/sphere.hpp"
    "include/hit_record.hpp"
    "include/kd_tree.hpp"
    "include/hittable.hpp"
    "include/mesh.hpp"
    "./include/texture.hpp"

)

target_include_directories(Geometry PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/src/common/include/"
    "${CMAKE_SOURCE_DIR}/src/math/include/"
)

set_property(TARGET Geometry PROPERTY CXX_STANDARD 23)

#target_include_directories(Geometry PUBLIC "./include/")

target_link_libraries(Geometry PUBLIC common Math)
