// CG_Raytracing.cpp : Defines the entry point for the application.
//

#include "CG_Raytracing.h"

#include <SDL3/SDL.h>
#include <GL/glew.h>

#include <format>
#include <iostream>

int main() {
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		std::println(std::cout, "SDL_Init error: {}", SDL_GetError());
		std::exit(1);
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	auto window = SDL_CreateWindow("Test", 1000, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
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

		SDL_GL_SwapWindow(window);
	}

	SDL_GL_DestroyContext(context);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
}
