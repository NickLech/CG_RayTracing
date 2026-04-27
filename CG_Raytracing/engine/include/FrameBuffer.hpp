#pragma once

#include <GLContext.hpp>
#include <Texture2D.hpp>

#include <expected>
#include <vector>
#include <unordered_map>

namespace cg_raytracing {
	/// <summary>
	/// Attachment point of a texture to
	/// a framebuffer. Note that only one
	/// texture can be bound to the 
	/// depth and/or stencil attachments
	/// at any given time.
	/// </summary>
	enum class FrameBufferAttachment {
		COLOR,         // Texture is used as color render target
		DEPTH,         // Texture is used as depth buffer
		STENCIL,       // Texture is used as stencil buffer
		DEPTH_STENCIL  // Texture is used both as depth and stencil buffer
	};

	enum class FrameBufferAttachmentDescriptorType {
		TEXTURE1D,
		TEXTURE2D,
		TEXTURE3D
	};

	struct FrameBufferAttachmentDescriptor {
		std::shared_ptr<TextureBase>        texture;
		uint32_t                            mip_level{};
		std::optional<uint32_t>             layer;
		FrameBufferAttachmentDescriptorType texture_type{};
		uint32_t                            attachment_type{};
	};

	class FrameBuffer {
	public :
		FrameBuffer(FrameBuffer&& _prev) noexcept;

		FrameBuffer& operator=(FrameBuffer&& _other) noexcept;

		~FrameBuffer();

		/// <summary>
		/// Create a new framebuffer, with no attachments
		/// </summary>
		/// <returns>Framebuffer or error</returns>
		static std::expected<FrameBuffer, GLError> CreateFramebuffer();

		std::optional<GLError> AddAttachment2D(std::shared_ptr<Texture2D> _attachment, FrameBufferAttachment _type, uint32_t _mip_level);

		std::expected<FrameBufferAttachmentDescriptor, GLError> GetColorAttachment(uint32_t _number) const;

		std::expected<FrameBufferAttachmentDescriptor, GLError> GetDepthAttachment() const;

		std::expected<FrameBufferAttachmentDescriptor, GLError> GetStencilAttachment() const;

		bool CheckCompletionStatus();

		std::optional<GLError> SetDrawBuffers(std::vector<uint32_t> _color_buffers);

	private :
		FrameBuffer();
		FrameBuffer(FrameBuffer const& _other) = delete;

		void AddAttachmentImpl(std::shared_ptr<TextureBase> _attachment, 
			FrameBufferAttachmentDescriptorType _texture_type, 
			uint32_t _mip_level, 
			std::optional<uint32_t> _layer,
			uint32_t _attachment_type);

	private :
		uint32_t                                                      m_fbo_handle;
		uint32_t                                                      m_curr_color_attachment;
		std::unordered_map<uint32_t, FrameBufferAttachmentDescriptor> m_color_attachments;
		std::optional<FrameBufferAttachmentDescriptor>                m_depth_attachment;
		std::optional<FrameBufferAttachmentDescriptor>                m_stencil_attachment;
		bool                                                          m_is_complete;
	};
}