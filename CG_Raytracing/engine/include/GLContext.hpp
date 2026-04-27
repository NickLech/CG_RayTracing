#pragma once

#include <cstdint>
#include <unordered_map>
#include <functional>

#include <Macros.hpp>
#include <GLError.hpp>

#include <TextureFormats.hpp>

namespace cg_raytracing {
	struct BufferRangeBinding {
		uint32_t target{};
		uint32_t index{};
		uint32_t buffer{};
		size_t offset{};
		size_t extent{};
	};

	struct ScissorBox {
		int32_t bottom_left_x{}, bottom_left_y{};
		int32_t w{}, h{};
	};

	struct ViewportBox {
		int32_t bottom_left_x{}, bottom_left_y{};
		int32_t w{}, h{};
	};

	struct ImageBinding {
		uint32_t unit{};
		uint32_t texture{};
		uint32_t mip_level{};
		bool layered{};
		int32_t layer{};
		ImageAccessFlags access{};
		TextureFormat format{};
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
		using SetContextFun = std::function<bool(void*, void*)>;

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

		void SetScissor(ScissorBox const& _scissor);
		void SetViewport(ViewportBox const& _viewport);

		void SetActiveTextureSlot(uint32_t _slot);
		void BindTexture(uint32_t _target, uint32_t _texture);

		void BindImage(ImageBinding const& _binding);

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
		ScissorBox                                       m_scissor;
		ViewportBox                                      m_viewport;
		uint32_t                                         m_active_texture_slot;
		Map<uint32_t, uint32_t>                          m_texture_bindings;
		Map<uint32_t, ImageBinding>                      m_image_bindings;
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

	/// <summary>
	/// Convert OpenGL error value to
	/// our representation
	/// </summary>
	/// <param name="_err">Error to convert</param>
	/// <returns>New error representation</returns>
	GLError ConvertGLErrorToEnum(uint32_t _err);
}