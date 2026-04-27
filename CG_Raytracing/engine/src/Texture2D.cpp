#include <Texture2D.hpp>

#include <GL/glew.h>

namespace cg_raytracing {
	Texture2D::Texture2D(Texture2D&& _prev) noexcept :
		Texture2D() {
		m_tex_handle    = _prev.m_tex_handle;
		m_mipmap_levels = _prev.m_mipmap_levels;
		m_width  = _prev.m_width;
		m_height = _prev.m_height;
		m_format = _prev.m_format;
		m_min_filter = _prev.m_min_filter;
		m_mag_filter = _prev.m_mag_filter;

		_prev.m_tex_handle = 0;
	}

	Texture2D& Texture2D::operator=(Texture2D&& _other) noexcept {
		this->~Texture2D();

		m_tex_handle    = _other.m_tex_handle;
		m_mipmap_levels = _other.m_mipmap_levels;
		m_width  = _other.m_width;
		m_height = _other.m_height;
		m_format = _other.m_format;
		m_min_filter = _other.m_min_filter;
		m_mag_filter = _other.m_mag_filter;

		_other.m_tex_handle = 0;

		return *this;
	}

	std::expected<Texture2D, GLError> Texture2D::CreateTexture(uint32_t _mip_levels, uint32_t _w, uint32_t _h, TextureFormat _format) {
		uint32_t tex_handle{};
		glCreateTextures(GL_TEXTURE_2D, 1, &tex_handle);

		if (0 == tex_handle) {
			return std::unexpected{ ConvertGLErrorToEnum(glGetError()) };
		}

		auto tex2d = Texture2D();
		tex2d.m_tex_handle = tex_handle;

		auto gl_format = GetOpenGLTextureFormatFromEnum(_format);
		if (!gl_format.has_value()) {
			return std::unexpected{ gl_format.error() };
		}
		glTextureStorage2D(tex_handle, _mip_levels, gl_format.value(), _w, _h);

		auto maybe_err = ConvertGLErrorToEnum(glGetError());
		if (GLError::OK != maybe_err) {
			return std::unexpected{ maybe_err };
		}

		tex2d.m_format = _format;
		tex2d.m_mipmap_levels = _mip_levels;
		tex2d.m_width = _w;
		tex2d.m_height = _h;

		return tex2d;
	}

	std::expected<std::shared_ptr<Texture2D>, GLError> Texture2D::CreateSharedTexture(uint32_t _mip_levels, uint32_t _w, uint32_t _h, TextureFormat _format) {
		auto tex_or_err = CreateTexture(_mip_levels, _w, _h, _format);
		if (!tex_or_err.has_value()) {
			return std::unexpected{ tex_or_err.error() };
		}
		return std::make_shared<Texture2D>(std::move(tex_or_err.value()));
	}

	std::expected<Texture2D, GLError> Texture2D::Clone() const {
		auto maybe_new_texture = CreateTexture(m_mipmap_levels, m_width, m_height, m_format);
		if (!maybe_new_texture.has_value()) {
			return std::unexpected{ maybe_new_texture.error() };
		}
		auto new_texture = std::move(maybe_new_texture.value());

		for (size_t level = 0; level < m_mipmap_levels; level++) {
			auto maybe_err = new_texture.CopyFromTexture(*this, level, 0, 0,
				level, 0, 0, GetMipLevelWidht(level), GetMipLevelHeight(level));
			if (maybe_err.has_value()) {
				return std::unexpected{ maybe_err.value() };
			}
		}
		
		return new_texture;
	}

	std::optional<GLError> Texture2D::CopyFromTexture(Texture2D const& _other, 
		uint32_t _src_mip_level, uint32_t _srcx, uint32_t _srcy, 
		uint32_t _dst_mip_level, uint32_t _dstx, uint32_t _dsty, 
		uint32_t _width, uint32_t _height) {
		glCopyImageSubData(
			_other.m_tex_handle, GL_TEXTURE_2D, _src_mip_level,
			_srcx, _srcy, 0,
			m_tex_handle, GL_TEXTURE_2D, _dst_mip_level,
			_dstx, _dsty, 0,
			_width, _height, 1);
		auto maybe_err = ConvertGLErrorToEnum(glGetError());
		if (GLError::OK != maybe_err) {
			return maybe_err;
		}
		return std::nullopt;
	}

	std::optional<GLError> Texture2D::CopyFromBuffer(const uint8_t* _data, 
		uint32_t _mip_level, 
		uint32_t _xoff, uint32_t _yoff, 
		uint32_t _width, uint32_t _height, 
		PixelFormat _pixel_format, PixelDataType _pixel_type) {
		auto maybe_format = GetOpenGLPixelFormatFromEnum(_pixel_format);
		if (!maybe_format.has_value()) {
			return maybe_format.error();
		}
		auto maybe_type = GetOpenGLDatatypeFromEnum(_pixel_type);
		if (!maybe_type.has_value()) {
			return maybe_type.error();
		}
		glTextureSubImage2D(m_tex_handle, _mip_level,
			_xoff, _yoff, _width, _height,
			maybe_format.value(), maybe_type.value(),
			(const void*)_data);
		auto maybe_err = ConvertGLErrorToEnum(glGetError());
		if (GLError::OK != maybe_err) {
			return maybe_err;
		}
		return std::nullopt;
	}

	std::optional<GLError> Texture2D::GetSubImage(uint8_t* _data, 
		uint32_t _buf_size_bytes, 
		uint32_t _mip_level, 
		uint32_t _xoff, uint32_t _yoff, 
		uint32_t _width, uint32_t _height, 
		PixelFormat _pixel_format, PixelDataType _pixel_type) {
		auto maybe_format = GetOpenGLPixelFormatFromEnum(_pixel_format);
		if (!maybe_format.has_value()) {
			return maybe_format.error();
		}
		auto maybe_type = GetOpenGLDatatypeFromEnum(_pixel_type);
		if (!maybe_type.has_value()) {
			return maybe_type.error();
		}
		glGetTextureSubImage(m_tex_handle, _mip_level, _xoff, _yoff, 0,
			_width, _height, 1,
			maybe_format.value(),
			maybe_type.value(),
			_buf_size_bytes,
			(void*)_data);
		auto maybe_err = ConvertGLErrorToEnum(glGetError());
		if (GLError::OK != maybe_err) {
			return maybe_err;
		}
		return std::nullopt;
	}

	std::optional<GLError> Texture2D::SetDownscaleFilter(SamplerFilter _filter) {
		auto maybe_filter = GetOpenGLFilteringFromEnum(_filter);
		if (!maybe_filter.has_value()) {
			return maybe_filter.error();
		}
		glTextureParameteri(m_tex_handle, GL_TEXTURE_MIN_FILTER, maybe_filter.value());
		auto maybe_err = ConvertGLErrorToEnum(glGetError());
		if (GLError::OK != maybe_err) {
			return maybe_err;
		}
		m_min_filter = _filter;
		return std::nullopt;
	}

	std::optional<GLError> Texture2D::SetUpscaleFilter(SamplerFilter _filter) {
		auto maybe_filter = GetOpenGLFilteringFromEnum(_filter);
		if (!maybe_filter.has_value()) {
			return maybe_filter.error();
		}
		glTextureParameteri(m_tex_handle, GL_TEXTURE_MAG_FILTER, maybe_filter.value());
		auto maybe_err = ConvertGLErrorToEnum(glGetError());
		if (GLError::OK != maybe_err) {
			return maybe_err;
		}
		m_mag_filter = _filter;
		return std::nullopt;
	}

	Texture2D::~Texture2D() {
		if (0 == m_tex_handle) {
			return;
		}
		glDeleteTextures(1, &m_tex_handle);
		m_tex_handle = 0;
	}

	void Texture2D::BindTexture(uint32_t _target) const {
		auto ctx = GetCurrentGLContext();
		ctx->BindTexture(_target, m_tex_handle);
	}
	
	std::optional<GLError> Texture2D::BindImage(ImageBinding _binding) const {
		auto maybe_format = GetOpenGLTextureFormatFromEnum(_binding.format);
		if (!maybe_format.has_value()) {
			return maybe_format.error();
		}
		auto maybe_access = GetOpenGLImageAccessFromEnum(_binding.access);
		if (!maybe_access.has_value()) {
			return maybe_access.error();
		}
		_binding.texture = m_tex_handle;
		auto ctx = GetCurrentGLContext();
		ctx->BindImage(_binding);
		return std::nullopt;
	}

	uint32_t Texture2D::GetPixelSizeBytes() const {
		return GetPixelSizeBytesFromEnum(m_format).value();
	}

	Texture2D::Texture2D() :
		m_tex_handle{},
		m_mipmap_levels{},
		m_width{},
		m_height{},
		m_format{},
		m_min_filter{SamplerFilter::NEAREST},
		m_mag_filter{SamplerFilter::NEAREST} {};
}