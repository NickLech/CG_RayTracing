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

	GLContextWrapper* GetCurrentGLContext() {
		return g_curr_ctx;
	}
}