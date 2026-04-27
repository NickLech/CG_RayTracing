#include <FrameBuffer.hpp>

#include <GL/glew.h>
#include <algorithm>

namespace cg_raytracing {
	FrameBuffer::FrameBuffer() :
		m_fbo_handle{},
		m_curr_color_attachment{},
		m_color_attachments{},
		m_depth_attachment{},
		m_stencil_attachment{},
		m_is_complete{}
	{ }


	FrameBuffer::FrameBuffer(FrameBuffer&& _prev) noexcept :
		m_fbo_handle{_prev.m_fbo_handle},
		m_curr_color_attachment{_prev.m_curr_color_attachment},
		m_color_attachments{std::move(_prev.m_color_attachments)},
		m_depth_attachment{std::move(_prev.m_depth_attachment)},
		m_stencil_attachment{std::move(_prev.m_stencil_attachment)},
		m_is_complete{_prev.m_is_complete} {
		_prev.m_fbo_handle = 0;
		_prev.m_curr_color_attachment = 0;
		_prev.m_is_complete = false;
	}

	FrameBuffer& FrameBuffer::operator=(FrameBuffer&& _other) noexcept {
		this->~FrameBuffer();

		m_fbo_handle = _other.m_fbo_handle;
		m_curr_color_attachment = _other.m_curr_color_attachment;
		m_color_attachments = std::move(_other.m_color_attachments);
		m_depth_attachment = std::move(_other.m_depth_attachment);
		m_stencil_attachment = std::move(_other.m_stencil_attachment);
		m_is_complete = _other.m_is_complete;

		_other.m_fbo_handle = 0;
		_other.m_curr_color_attachment = 0;
		_other.m_is_complete = false;
		return *this;
	}

	FrameBuffer::~FrameBuffer() {
		if (0 == m_fbo_handle) {
			return;
		}
		glDeleteFramebuffers(1, &m_fbo_handle);
		m_fbo_handle = 0;
		m_is_complete = false;
	}

	std::expected<FrameBuffer, GLError> FrameBuffer::CreateFramebuffer() {
		uint32_t fbo_handle{};
		glCreateFramebuffers(1, &fbo_handle);

		auto maybe_err = ConvertGLErrorToEnum(glGetError());
		if (GLError::OK != maybe_err) {
			return std::unexpected{ maybe_err };
		}

		auto new_framebuf = FrameBuffer();
		new_framebuf.m_fbo_handle = fbo_handle;

		return new_framebuf;
	}

	static std::expected<uint32_t, GLError> GetOpenGLFramebufferAttachmentFromEnum(FrameBufferAttachment _attachment) {
		switch (_attachment)
		{
		case FrameBufferAttachment::COLOR:
			return GL_COLOR_ATTACHMENT0;
		case FrameBufferAttachment::DEPTH:
			return GL_DEPTH_ATTACHMENT;
		case FrameBufferAttachment::STENCIL:
			return GL_STENCIL_ATTACHMENT;
		case FrameBufferAttachment::DEPTH_STENCIL:
			return GL_DEPTH_STENCIL_ATTACHMENT;
		}

		return std::unexpected{ GLError::INVALID_ATTACHMENT };
	}

	std::optional<GLError> FrameBuffer::AddAttachment2D(std::shared_ptr<Texture2D> _attachment, FrameBufferAttachment _type, uint32_t _mip_level) {
		auto maybe_attachment = GetOpenGLFramebufferAttachmentFromEnum(_type);
		if (!maybe_attachment.has_value()) {
			return maybe_attachment.error();
		}

		auto attachment = maybe_attachment.value();
		if (GL_COLOR_ATTACHMENT0 == attachment) {
			attachment = GL_COLOR_ATTACHMENT0 + m_curr_color_attachment;
			m_curr_color_attachment++;
		}

		glNamedFramebufferTexture(m_fbo_handle, attachment, _attachment->GetTextureHandle(), _mip_level);
		auto maybe_err = ConvertGLErrorToEnum(glGetError());
		if (GLError::OK != maybe_err) {
			return maybe_err;
		}

		AddAttachmentImpl(std::dynamic_pointer_cast<TextureBase>(_attachment), 
			FrameBufferAttachmentDescriptorType::TEXTURE2D, _mip_level, std::nullopt, attachment);
		return std::nullopt;
	}

	std::expected<FrameBufferAttachmentDescriptor, GLError> FrameBuffer::GetColorAttachment(uint32_t _number) const {
		if (m_color_attachments.size() < _number) {
			return std::unexpected{ GLError::INVALID_ATTACHMENT };
		}
		return m_color_attachments.find(_number)->second;
	}

	std::expected<FrameBufferAttachmentDescriptor, GLError> FrameBuffer::GetDepthAttachment() const {
		if (!m_depth_attachment.has_value()) {
			return std::unexpected{ GLError::INVALID_ATTACHMENT };
		}
		return m_depth_attachment.value();
	}

	std::expected<FrameBufferAttachmentDescriptor, GLError> FrameBuffer::GetStencilAttachment() const {
		if (!m_stencil_attachment.has_value()) {
			return std::unexpected{ GLError::INVALID_ATTACHMENT };
		}
		return m_stencil_attachment.value();
	}

	bool FrameBuffer::CheckCompletionStatus() {
		auto status = glCheckNamedFramebufferStatus(m_fbo_handle, GL_FRAMEBUFFER);
		m_is_complete = status == GL_FRAMEBUFFER_COMPLETE;
		return m_is_complete;
	}

	std::optional<GLError> FrameBuffer::SetDrawBuffers(std::vector<uint32_t> _color_buffers) {
		for (auto index : _color_buffers) {
			if (!m_color_attachments.contains(index)) {
				return GLError::INVALID_ATTACHMENT;
			}
		}

		if (!m_is_complete) {
			return GLError::FRAMEBUFFER_INCOMPLETE;
		}

		std::transform(_color_buffers.cbegin(), _color_buffers.cend(),
			_color_buffers.begin(), [](uint32_t _index) { return GL_COLOR_ATTACHMENT0 + _index; });
		glDrawBuffers((GLsizei)_color_buffers.size(), _color_buffers.data());
		auto maybe_err = ConvertGLErrorToEnum(glGetError());
		if (GLError::OK != maybe_err) {
			return maybe_err;
		}

		return std::nullopt;
	}

	void FrameBuffer::AddAttachmentImpl(
		std::shared_ptr<TextureBase> _attachment, 
		FrameBufferAttachmentDescriptorType _texture_type,
		uint32_t _mip_level, 
		std::optional<uint32_t> _layer,
		uint32_t _attachment_type) {
		FrameBufferAttachmentDescriptor descriptor{};
		descriptor.attachment_type = _attachment_type;
		descriptor.layer = _layer;
		descriptor.mip_level = _mip_level;
		descriptor.texture_type = _texture_type;
		descriptor.texture = _attachment;
		if (GL_DEPTH_ATTACHMENT == _attachment_type) {
			m_depth_attachment = descriptor;
		}
		else if (GL_STENCIL_ATTACHMENT == _attachment_type) {
			m_stencil_attachment = descriptor;
		}
		else if (GL_DEPTH_STENCIL_ATTACHMENT == _attachment_type) {
			descriptor.attachment_type = GL_DEPTH_ATTACHMENT;
			auto descriptor2 = descriptor;
			descriptor2.attachment_type = GL_STENCIL_ATTACHMENT;
			m_depth_attachment = descriptor;
			m_stencil_attachment = descriptor2;
		}
		else {
			m_color_attachments[m_curr_color_attachment - 1] = descriptor;
		}
	}
}