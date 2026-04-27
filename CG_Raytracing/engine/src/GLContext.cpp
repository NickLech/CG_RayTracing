#include <GLContext.hpp>
#include <Utility.hpp>

#include <GL/glew.h>

namespace cg_raytracing {
	thread_local GLContextWrapper* g_curr_ctx = nullptr;

	GLContextWrapper GLContextWrapper::CreateWrapper(void* _gl_context, SetContextFun _set_ctx_function) {
		return GLContextWrapper{ .m_handle = _gl_context, .m_set_context = _set_ctx_function };
	}

	[[nodiscard]] bool GLContextWrapper::MakeCurrent(void* _window) {
		if (GetCurrentGLContext() == this) {
			return true;
		}
		if (!m_set_context(_window, m_handle)) {
			return false;
		}
		g_curr_ctx = this;
		return true;
	}

	void GLContextWrapper::BindProgram(uint32_t _program) {
		if (m_curr_program == _program) {
			return;
		}
		glUseProgram(_program);
		m_curr_program = _program;
	}

	void GLContextWrapper::SetBlendEnable(bool _blend) {
		if (m_enable_blend == _blend) {
			return;
		}
		if (_blend) {
			glEnable(GL_BLEND);
		}
		else {
			glDisable(GL_BLEND);
		}
		m_enable_blend = _blend;
	}

	void GLContextWrapper::SetScissorEnable(bool _scissor) {
		if (m_enable_scissor == _scissor) {
			return;
		}
		if (_scissor) {
			glEnable(GL_SCISSOR_TEST);
		}
		else {
			glDisable(GL_SCISSOR_TEST);
		}
		m_enable_scissor = _scissor;
	}

	void GLContextWrapper::BindBuffer(uint32_t _target, uint32_t _buf_id) {
		if (!m_bound_buffers.contains(_target)) {
			m_bound_buffers[_target] = _buf_id;
			glBindBuffer(_target, _buf_id);
			return;
		}

		if (m_bound_buffers[_target] == _buf_id) {
			return;
		}

		glBindBuffer(_target, _buf_id);
		m_bound_buffers[_target] = _buf_id;
	}

	void GLContextWrapper::BindVao(uint32_t _vao) {
		if (m_bound_vao == _vao) {
			return;
		}

		glBindVertexArray(_vao);
		m_bound_vao = _vao;
	}

	void GLContextWrapper::BindBufferRange(BufferRangeBinding const& _binding) {
		if (!m_range_bindings.contains(_binding.target)) {
			Map<uint32_t, BufferRangeBinding> bind_map{};
			bind_map[_binding.index] = _binding;
			m_range_bindings[_binding.target] = std::move(bind_map);
			glBindBufferRange(_binding.target, _binding.index,
				_binding.buffer, _binding.offset,
				_binding.extent);
			return;
		}

		if (!m_range_bindings[_binding.target].contains(_binding.index)) {
			m_range_bindings[_binding.target][_binding.index] = _binding;
			glBindBufferRange(_binding.target, _binding.index,
				_binding.buffer, _binding.offset,
				_binding.extent);
			return;
		}

		auto const& old_binding = m_range_bindings[_binding.target][_binding.index];
		if (old_binding.buffer == _binding.buffer &&
			old_binding.extent == _binding.extent &&
			old_binding.offset == _binding.offset) {
			return;
		}

		m_range_bindings[_binding.target][_binding.index] = _binding;
		glBindBufferRange(_binding.target, _binding.index,
			_binding.buffer, _binding.offset,
			_binding.extent);
	}

	void GLContextWrapper::SetScissor(ScissorBox const& _scissor) {
		if (_scissor.bottom_left_x == m_scissor.bottom_left_x &&
			_scissor.bottom_left_y == m_scissor.bottom_left_y &&
			_scissor.w == m_scissor.w &&
			_scissor.h == m_scissor.h) {
			return;
		}

		m_scissor = _scissor;
		glScissor(_scissor.bottom_left_x, _scissor.bottom_left_y,
			_scissor.w, _scissor.h);
	}

	void GLContextWrapper::SetViewport(ViewportBox const& _viewport) {
		if (_viewport.bottom_left_x == m_viewport.bottom_left_x &&
			_viewport.bottom_left_y == m_viewport.bottom_left_y &&
			_viewport.w == m_viewport.w &&
			_viewport.h == m_viewport.h) {
			return;
		}

		m_viewport = _viewport;
		glViewport(_viewport.bottom_left_x, _viewport.bottom_left_y,
			_viewport.w, _viewport.h);
	}

	void GLContextWrapper::SetActiveTextureSlot(uint32_t _slot) {
		if (m_active_texture_slot == _slot) {
			return;
		}

		m_active_texture_slot = _slot;
		glActiveTexture(_slot);
	}

	void GLContextWrapper::BindTexture(uint32_t _target, uint32_t _texture) {
		if (!m_texture_bindings.contains(_target)) {
			m_texture_bindings[_target] = _texture;
			glBindTexture(_target, _texture);
			return;
		}

		if (m_texture_bindings[_target] == _texture) {
			return;
		}

		m_texture_bindings[_target] = _texture;
		glBindTexture(_target, _texture);
	}

	void GLContextWrapper::BindImage(ImageBinding const& _binding) {
		if (!m_image_bindings.contains(_binding.unit)) {
			m_image_bindings[_binding.unit] = _binding;
			glBindImageTexture(_binding.unit, _binding.texture, _binding.mip_level,
				_binding.layered, _binding.layer,
				GetOpenGLImageAccessFromEnum(_binding.access).value(),
				GetOpenGLTextureFormatFromEnum(_binding.format).value());
			return;
		}

		/*
		uint32_t unit{};
		uint32_t texture{};
		uint32_t mip_level{};
		bool layered{};
		int32_t layer{};
		ImageAccessFlags access{};
		TextureFormat format{};
		*/

		auto const& old_binding = m_image_bindings[_binding.unit];
		if (_binding.texture == old_binding.texture &&
			_binding.mip_level == old_binding.mip_level &&
			_binding.layered == old_binding.layered &&
			_binding.layer == old_binding.layer &&
			_binding.access == old_binding.access &&
			_binding.format == old_binding.format) {
			return;
		}

		m_image_bindings[_binding.unit] = _binding;
		glBindImageTexture(_binding.unit, _binding.texture, _binding.mip_level,
			_binding.layered, _binding.layer,
			GetOpenGLImageAccessFromEnum(_binding.access).value(),
			GetOpenGLTextureFormatFromEnum(_binding.format).value());
	}

	GLContextWrapper* GetCurrentGLContext() {
		return g_curr_ctx;
	}

	GLError ConvertGLErrorToEnum(uint32_t _err) {
		switch (_err)
		{
		case GL_NO_ERROR:
			return GLError::OK;
		case GL_INVALID_ENUM:
			return GLError::INVALID_ENUM;
		case GL_INVALID_VALUE:
			return GLError::INVALID_VALUE;
		case GL_INVALID_OPERATION:
			return GLError::INVALID_OPERATION;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			return GLError::INVALID_FRAMEBUFFER_OPERATION;
		case GL_OUT_OF_MEMORY:
			return GLError::OUT_OF_MEMORY;
		case GL_STACK_OVERFLOW:
			return GLError::STACK_OVERFLOW;
		case GL_STACK_UNDERFLOW:
			return GLError::STACK_UNDERFLOW;
		default:
			return GLError::UNKNOWN;
		}

		common::Unreachable();
		return GLError::OK;
	}
}