// CG_Raytracing.cpp : Defines the entry point for the application.
//

#include "CG_Raytracing.h"

#include <SDL3/SDL.h>
#include <GL/glew.h>

#include <Shader.hpp>

#include <format>
#include <iostream>
#include <filesystem>

void __stdcall DebugCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam) {
	std::string msg{ message, (size_t)length };
	std::println(std::cout, "{}", message);
}

int main() {
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		std::println(std::cout, "SDL_Init error: {}", SDL_GetError());
		std::exit(1);
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

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

	SDL_GL_MakeCurrent(window, context);

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

	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_BLEND);

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(DebugCallback, nullptr);

	bool close = false;
	while (!close) {
		SDL_Event ev{};
		while (SDL_PollEvent(&ev)) {
			switch (ev.type)
			{
			case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
				close = true;
				break;
			default:
				break;
			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1.f, 1.f, 1.f, 1.f);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader.Bind();
		//This will cause GL_INVALID_OPERATION since
		//no VAO is bound
		glDrawArrays(GL_TRIANGLES, 0, 6);

		auto sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

		auto flag = true;
		while (flag)
		{
			auto res = glClientWaitSync(
				sync, GL_SYNC_FLUSH_COMMANDS_BIT,
				10000000
			);

			if (res == GL_ALREADY_SIGNALED ||
				res == GL_CONDITION_SATISFIED)
				flag = false;
		}

		glDeleteSync(sync);

		SDL_GL_SwapWindow(window);
	}

	SDL_GL_DestroyContext(context);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
}
