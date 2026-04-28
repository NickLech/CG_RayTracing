// CG_Raytracing.cpp : Defines the entry point for the application.
//

#include "CG_Raytracing.h"

#include <SDL3/SDL.h>
#include <GL/glew.h>

#include <Shader.hpp>
#include <GLContext.hpp>
#include <VertexBuffer.hpp>
#include <IndexBuffer.hpp>
#include <Texture2D.hpp>

#include <format>
#include <iostream>
#include <filesystem>
#include <bit>
#include <ctime>
#include <print>

void DebugCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam) {
	std::string msg{ message, (size_t)length };
	std::println(std::cout, "{}", message);
}

struct Vertex2D {
#pragma pack(push, 1)
	float x, y;
	float u, v;
#pragma pack(pop)

	std::vector<cg_raytracing::VertexAttribute> attributes() const {
		return {
			cg_raytracing::VertexAttribute{ cg_raytracing::VertexAttributeType::FLOAT, (char*)&x - (char*)this },
			cg_raytracing::VertexAttribute{ cg_raytracing::VertexAttributeType::FLOAT, (char*)&y - (char*)this },
			cg_raytracing::VertexAttribute{ cg_raytracing::VertexAttributeType::FLOAT, (char*)&u - (char*)this },
			cg_raytracing::VertexAttribute{ cg_raytracing::VertexAttributeType::FLOAT, (char*)&v - (char*)this }
		};
	}
};

int main() {
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		std::println(std::cout, "SDL_Init error: {}", SDL_GetError());
		std::exit(1);
	}

	// Set double-buffering on the swapchain (render to one buffer, while another is presented by the window)
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	// Set OpenGL 4.6
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	// Disable compatibility profile sheganigans
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	// SDL_CreateWindow requires SDL_WINDOW_OPENGL to use OpenGL
	auto window = SDL_CreateWindow("Test", 1000, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_HIGH_PIXEL_DENSITY);
	if (window == nullptr) {
		std::println(std::cout, "SDL_CreateWindow error: {}", SDL_GetError());
		std::exit(1);
	}

	auto context = SDL_GL_CreateContext(window);
	if (context == nullptr) {
		std::println(std::cout, "SDL_GL_CreateContext error: {}", SDL_GetError());
		std::exit(1);
	}

	auto gl_ctx = cg_raytracing::GLContextWrapper::CreateWrapper(context, [](void* window, void* ctx) {
		// To be used, generally a context must be bound to a window
		// This lambda does exactly that
		return SDL_GL_MakeCurrent((SDL_Window*)window, (SDL_GLContext)ctx);
	});
	if (!gl_ctx.MakeCurrent((void*)window)) {
		std::println(std::cout, "SDL_GL_MakeCurrent() error: {}", SDL_GetError());
		std::exit(1);
	}

	glewExperimental = true;
	auto error = glewInit();
	if (error != GLEW_OK) {
		std::println(std::cout, "glewInit error: {}", (const char*)glewGetErrorString(error));
		std::exit(1);
	}

	auto version_string = glewGetString(GLEW_VERSION);
	std::println(std::cout, "GLEW  version {}", (const char*)version_string);

	int32_t major{}, minor{};
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	std::println(std::cout, "OpenGL {}.{}", major, minor);

	auto work_dir = std::filesystem::current_path();
	std::println(std::cout, "Working directory: {}", work_dir.string());

	using ShaderStage = cg_raytracing::Shader::ShaderStage;
	auto shader = cg_raytracing::Shader::CreateShaderFromFiles({ 
		{"./assets/main.vert", ShaderStage::VERTEX},
		{"./assets/main.frag", ShaderStage::FRAGMENT}
	}).value();

	// Disable alpha-blending
	gl_ctx.SetBlendEnable(false);
	// Disable scissor (e.g. clipping outside a specific rect)
	gl_ctx.SetScissorEnable(false);

	// https://wikis.khronos.org/opengl/Debug_Output
	// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDebugMessageCallback.xhtml
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(DebugCallback, nullptr);

	auto vert_buf = cg_raytracing::VertexBuffer::CreateVertexBuffer().value();
	vert_buf.AddBuffer<Vertex2D>(0, 4); // Add a single buffer, containing per-vertex data,
	                                    // reserve space for 4 vertices

	// Index buffer with size for 6 indices
	auto index_buf = cg_raytracing::IndexBuffer::CreateIndexBuffer(6).value();

	// 4 Vertices:
	// X, Y and U, V
	// X, Y are in screen coordinates (in [-1.0, 1.0])
	// U, V are in texture coordinates (in [0.0, 1.0])
	constexpr Vertex2D VERTICES[4] = { 
		{ -1.0, 1.0, 0.0, 1.0 },
		{ 1.0, 1.0, 1.0, 1.0 },
		{ -1.0, -1.0, 0.0, 0.0 },
		{ 1.0, -1.0, 1.0, 0.0 }
	};

	// Indices in the vertex buffer to draw a quad
	constexpr float INDICES[6] = { 0, 1, 2, 2, 1, 3 };

	for (size_t i = 0; i < 4; i++) {
		vert_buf.PushVertexDataTyped(0, VERTICES[i]);
	}

	for (size_t i = 0; i < 6; i++) {
		index_buf.PushIndex(INDICES[i]);
	}

	// Use the two buffers
	// and the shader
	vert_buf.Bind();
	index_buf.Bind();
	shader.Bind();

	srand(time(0));

	auto tex = cg_raytracing::Texture2D::CreateTexture(1, 640, 480, cg_raytracing::TextureFormat::RGB8).value();
	tex.SetUpscaleFilter(cg_raytracing::SamplerFilter::LINEAR);
	tex.SetDownscaleFilter(cg_raytracing::SamplerFilter::LINEAR);
	std::vector<uint8_t> temp_buf{};
	temp_buf.resize(tex.GetSizeBytes());
	for (auto& val : temp_buf) {
		val = (uint8_t)(rand() % 256);
	}
	tex.CopyFromBuffer(temp_buf.data(), 0, 0, 0, tex.GetWidth(), tex.GetHeight(),
		cg_raytracing::PixelFormat::RGB, cg_raytracing::PixelDataType::UNSIGNED_BYTE);
	tex.BindTexture(GL_TEXTURE_2D);

	bool close = false;
	while (!close) {
		SDL_Event ev{};
		while (SDL_PollEvent(&ev)) {
			switch (ev.type)
			{
			case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
				close = true;
				break;
			case SDL_EVENT_WINDOW_RESIZED: {
				int32_t w{}, h{};
				SDL_GetWindowSize(window, &w, &h);
				gl_ctx.SetViewport({
					.bottom_left_x = 0,
					.bottom_left_y = 0,
					.w = w,
					.h = h
				});
			} break;
			default:
				break;
			}
		}

		// glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// Clear the current framebuffer
		// Clearing can be divided in color buffer, depth buffer and stencil buffer
		glClearColor(1.f, 1.f, 1.f, 1.f); // Specify clear color for color buffer
		glClearDepth(1.0f);               // Specify clear for depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Effectively clear color and depth
		
		// Draw indexed vertices:
		// 6 triangles, drawn using 6 indices
		// from the bound index buffer, starting
		// from offset 0 in the index buffer

		// In short, instead of using directly the vertex buffer
		// for drawing, which would require replicating the vertex
		// data for adjacent vertices, we write that data once
		// in the vertex buffer, and then we write that vertex
		// index multiple times in the index buffer. This
		// can reduce the memory usage by a lot, especially
		// if each vertex contains a lot of data

		// Example: a vertex contains the following data
		// X, Y, Z coordinates -> 12 bytes
		// U, V -> 8 bytes
		// Vertex normal -> 12 bytes
		// (vertex might also contain base color and so on...)
		// For a total of 32 bytes, for a quad we have 6
		// vertices -> 192 bytes
		// But what if we use a index buffer ?
		// 4 unique vertices -> 128 bytes
		// 6 indices, let's say 4 bytes each -> 24 bytes
		// In total 152 bytes
		// 40 bytes less
		// And this is only a very simple example
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void*)0);

		SDL_GL_SwapWindow(window);
	}

	SDL_GL_DestroyContext(context);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
}
