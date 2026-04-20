add_subdirectory("./app")
add_subdirectory("./engine")
add_subdirectory("./common")

target_link_libraries(CG_Raytracing PUBLIC RaytracingEngine)
target_link_libraries(CG_Raytracing PUBLIC common)

target_link_libraries(RaytracingEngine PUBLIC common)

set_property(TARGET CG_Raytracing PROPERTY DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")