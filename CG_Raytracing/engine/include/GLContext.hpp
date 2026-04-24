#pragma once

#include <cstdint>
#include <unordered_map>

#include <Macros.hpp>

namespace cg_raytracing {
	struct BufferRangeBinding {
		uint32_t target{};
		uint32_t index{};
		uint32_t buffer{};
		size_t offset{};
		size_t extent{};
	};
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

		/// All this functions are considered low level, no errors are returned
		/// if an API call fails

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

		void BindBuffer(uint32_t _target, uint32_t _buf_id);
		void BindVao(uint32_t _vao);

		void BindBufferRange(BufferRangeBinding const& _binding);

		template <typename Key, typename Value>
		using Map = std::unordered_map<Key, Value>;

		//Opaque handle to the context
		void*                                            m_handle;
		//Opaque function pointer to set the current context
		SetContextFun			                         m_set_context;
		uint32_t				                         m_curr_program;
		bool					                         m_enable_blend;
		bool					                         m_enable_scissor;
		Map<uint32_t, uint32_t>                          m_bound_buffers;
		uint32_t				                         m_bound_vao;
		Map<uint32_t, Map<uint32_t, BufferRangeBinding>> m_range_bindings;
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

	/////////////////////////////////////////////

	/*
	  Enum used to represent underlying OpenGL errors
	  and also our errors
	* GL_NO_ERROR
	* GL_INVALID_ENUM
	* GL_INVALID_VALUE
	* GL_INVALID_OPERATION
	* GL_INVALID_FRAMEBUFFER_OPERATION
	* GL_OUT_OF_MEMORY
	* GL_STACK_UNDERFLOW
	* GL_STACK_OVERFLOW
	*/
	enum class GLError {
		OK = 0,                        // Autoesplicative
		INVALID_ENUM,                  // An enumerated argument (such as the format of a texture) is not valid
		INVALID_VALUE,                 // Numeric value (e.g. size of a texture) is out of range
		INVALID_OPERATION,             // May be a lot of different things
		INVALID_FRAMEBUFFER_OPERATION, // Attempting to use a framebuffer in an invalid state
		OUT_OF_MEMORY,				   // Autoesplicative
		STACK_UNDERFLOW,               // Autoesplicative
		STACK_OVERFLOW,                // Autoesplicative
		MAP_FAILED_BUFFER_OVERFLOW,    // Attempted to map a buffer outside its range
		MAP_FAILED_INVALID_PROTS,      // Attempted to map a buffer with invalid protections
		UNKNOWN,                       // IDK
		READ_FAILED_BUFFER_MAPPED,     // Attempting GL read buffer while the buffer is mapped without the "map persistent" bit
		READ_FAILED_BUFFER_OVERFLOW,   // Autoesplicative
		WRITE_FAILED_BUFFER_MAPPED,	   // Same as for buffer read
		WRITE_FAILED_BUFFER_OVERFLOW,  // Same as for buffer read
		RESIZE_FAILED_BUFFER_MAPPED,   // Buffer was CPU mapped while resizing
		PUSH_FAILED_INVALID_BUFFER,    // Attempting to push data to invalid buffer inside a vertex buffer
		PUSH_FAILED_TYPE_MISMATCH,     // Attempting to push data to buffer inside a vertex buffer with an incorrect type
		PUSH_FAILED_SIZE_MISMATCH,     // Attempting to push data to buffer inside a vertex buffer with an incorrect size
		PUSH_FAILED_BUFFER_OVERFLOW,   // Buffer overflow while appending to vertex buffer
		COPY_FAILED_BUFFER_MAPPED,     // Attempting to copy a buffer to another while one of them was mapped or they do not have the PERSISTENT flag set
		CLEAR_FAILED_INVALID_BUFFER,   // 
		RESIZE_FAILED_MISSING_BUFFERS, // Attempting to resize vertex buffer without specifying all sub-buffers
		UNIFORM_NOT_FOUND              // glGetUniformLocation returned -1
	};

	/// <summary>
	/// Convert OpenGL error value to
	/// our representation
	/// </summary>
	/// <param name="_err">Error to convert</param>
	/// <returns>New error representation</returns>
	GLError ConvertGLErrorToEnum(uint32_t _err);
}