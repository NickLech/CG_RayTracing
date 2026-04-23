set(ENGINE_SOURCES 
    "src/Shader.cpp"
    "src/GLContext.cpp" 
    "src/GpuBuffer.cpp" 
    "src/VertexBuffer.cpp" 
    "src/IndexBuffer.cpp")

set(ENGINE_INCLUDES 
    "include/Shader.hpp"
    "include/GLContext.hpp"
    "include/GpuBuffer.hpp"
    "include/VertexBuffer.hpp"
    "include/IndexBuffer.hpp")

add_library(RaytracingEngine STATIC ${ENGINE_INCLUDES} ${ENGINE_SOURCES})
set_property(TARGET RaytracingEngine PROPERTY CXX_STANDARD 23)

target_include_directories(RaytracingEngine PUBLIC "./include")

if(WIN32)
	target_link_libraries(RaytracingEngine PUBLIC libglew_shared)
elseif(UNIX)
	target_link_libraries(RaytracingEngine PRIVATE 
        GLEW::GLEW 
        OpenGL::GL
    )
endif()