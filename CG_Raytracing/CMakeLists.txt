include(FetchContent)

add_executable (CG_Raytracing "CG_Raytracing.cpp" "CG_Raytracing.h")
set_property(TARGET CG_Raytracing PROPERTY CXX_STANDARD 23)

if(WIN32)
	FetchContent_Declare(
		SDL3
		GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
		GIT_TAG "origin/main"
	)
	FetchContent_MakeAvailable(SDL3)

	FetchContent_Declare(
		glew
		GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
		GIT_TAG "origin/master"
	)
	FetchContent_MakeAvailable(glew)
	#add_subdirectory("${glew_SOURCE_DIR}/build/cmake" "${glew_BINARY_DIR}" SYSTEM)
	set(glew-cmake_BUILD_STATIC ON)
	set(ONLY_LIBS ON)
endif(WIN32)

target_link_libraries(CG_Raytracing PUBLIC SDL3::SDL3-shared)
target_link_libraries(CG_Raytracing PUBLIC libglew_static)
