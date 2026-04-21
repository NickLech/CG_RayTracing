#pragma once

#include <cstdint>

#include <Macros.hpp>

namespace cg_raytracing {
	/// <summary>
	/// Wrapper for the OpenGL context state.
	/// Use this to bind buffers/programs/texture
	/// and many other things.
	/// <para/>
	/// It should prevent unnecessary state
	/// changes
	/// <para>By the OpenGL threading model, 
	/// a context cannot be bound 
	/// to multiple threads at the same
	/// time</para>
	/// <para>The wrapper does not manage the context's lifetime nor the bound objects, destroy everything manually!</para>
	/// </summary>
	struct GLContextWrapper {
		using SetContextFun = bool(__cdecl*)(void* _window, void* _context);

		/// <summary>
		/// Create a wrapper for the specified context
		/// </summary>
		/// <param name="_gl_context">OpenGL context</param>
		/// <param name="_set_ctx_function">Function pointer to set the context (e.g. use SDL_GL_MakeCurrent)</param>
		/// <returns>The wrapper</returns>
		static GLContextWrapper CreateWrapper(void* _gl_context, SetContextFun _set_ctx_function);

		/// <summary>
		/// Returns the OpenGL context
		/// </summary>
		/// <returns>The OpenGL context</returns>
		FORCE_INLINE void* GetHandle() const {
			return m_handle;
		}
		
		/// <summary>
		/// Make context current with the
		/// specified window
		/// </summary>
		/// <param name="_window">Opaque window pointer</param>
		/// <returns>If the binding was successfull</returns>
		[[nodiscard]] bool MakeCurrent(void* _window);

		/// <summary>
		/// Bind the given program handle to 
		/// the context
		/// </summary>
		/// <param name="_program">Program handle</param>
		void BindProgram(uint32_t _program);

		void SetBlendEnable(bool _blend);
		void SetScissorEnable(bool _scissor);

		FORCE_INLINE uint32_t GetCurrentProgram() const {
			return m_curr_program;
		}

		//Opaque handle to the context
		void*         m_handle;
		//Opaque function pointer to set the current context
		SetContextFun m_set_context;
		uint32_t      m_curr_program;
		bool          m_enable_blend;
		bool          m_enable_scissor;
	};

	/// <summary>
	/// Only one context can be bound
	/// in a given thread. This is the 
	/// current context bound in the thread
	/// </summary>
	extern thread_local GLContextWrapper* g_curr_ctx;

	/// <summary>
	/// Retrieve the current
	/// thread's context
	/// </summary>
	/// <returns>The context</returns>
	GLContextWrapper* GetCurrentGLContext();
}