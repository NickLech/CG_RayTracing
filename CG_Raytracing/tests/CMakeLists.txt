add_executable(UnitTests 
    "./test_main.cpp"
    "./math/test_Vec3.cpp"
    "./geometry/test_obj_loader.cpp"
    "./geometry/test_mesh.cpp"
    "geometry/test_kd_tree.cpp"
    "material/test_material_scatter.cpp"
)

target_link_libraries(UnitTests PRIVATE Math Geometry)

target_include_directories(UnitTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/src/geometry/include"
)

if(WIN32)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.14.0
    )
    FetchContent_MakeAvailable(Catch2)
elseif(UNIX)
    find_package(Catch2 3 REQUIRED)
endif()

target_link_libraries(UnitTests PRIVATE Catch2::Catch2WithMain
Geometry PUBLIC common Math)


set_property(TARGET UnitTests PROPERTY CXX_STANDARD 23)

include(CTest)
include(Catch)
# catch_discover_tests(UnitTests)
catch_discover_tests(UnitTests
    PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
