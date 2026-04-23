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