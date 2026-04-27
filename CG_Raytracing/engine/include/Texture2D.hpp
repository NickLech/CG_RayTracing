#pragma once

#include <TextureBase.hpp>
#include <GLContext.hpp>
#include <TextureFormats.hpp>
#include <SamplerParams.hpp>

#include <expected>
#include <optional>
#include <memory>

namespace cg_raytracing {
	class Texture2D : public TextureBase {
	public :
		Texture2D(Texture2D&& _prev) noexcept;

		Texture2D& operator=(Texture2D&& _other) noexcept;

		/// <summary>
		/// Create a 2D texture
		/// </summary>
		/// <param name="_mip_levels">Number of mipmap levels</param>
		/// <param name="_w">Width of first level</param>
		/// <param name="_h">Height of first level</param>
		/// <param name="_format">Texture format</param>
		/// <returns>Texture or error</returns>
		static std::expected<Texture2D, GLError> CreateTexture(uint32_t _mip_levels, uint32_t _w, uint32_t _h, TextureFormat _format);

		/// <summary>
		/// Utility method: same as CreateTexture but texture is wrapped
		/// in a shared_ptr
		/// </summary>
		/// <param name="_mip_levels"></param>
		/// <param name="_w"></param>
		/// <param name="_h"></param>
		/// <param name="_format"></param>
		/// <returns></returns>
		static std::expected<std::shared_ptr<Texture2D>, GLError> CreateSharedTexture(uint32_t _mip_levels, uint32_t _w, uint32_t _h, TextureFormat _format);

		/// <summary>
		/// Clone this texture (e.g. create a new handle and
		/// copy all mipmap levels). Bound states are not
		/// copied (e.g. if this texture is bound to
		/// a texture or image unit)
		/// </summary>
		/// <returns>Clone or error</returns>
		std::expected<Texture2D, GLError> Clone() const;

		std::optional<GLError> CopyFromTexture(Texture2D const& _other, uint32_t _src_mip_level, uint32_t _srcx, uint32_t _srcy,
			uint32_t _dst_mip_level, uint32_t _dstx, uint32_t _dsty,
			uint32_t _width, uint32_t _height);

		std::optional<GLError> CopyFromBuffer(const uint8_t* _data, uint32_t _mip_level,
			uint32_t _xoff, uint32_t _yoff,
			uint32_t _width, uint32_t _height,
			PixelFormat _pixel_format,
			PixelDataType _pixel_type);

		std::optional<GLError> GetSubImage(uint8_t* _data, uint32_t _buf_size_bytes,
			uint32_t _mip_level,
			uint32_t _xoff, uint32_t _yoff,
			uint32_t _width, uint32_t _height,
			PixelFormat _pixel_format,
			PixelDataType _pixel_type);

		std::optional<GLError> SetDownscaleFilter(SamplerFilter _filter);
		std::optional<GLError> SetUpscaleFilter(SamplerFilter _filter);

		~Texture2D() override;

		/// <summary>
		/// Bind this texture to a normal texture
		/// unit as _target
		/// </summary>
		/// <param name="_target">Target of the binding</param>
		void BindTexture(uint32_t _target) const;

		/// <summary>
		/// Bind this texture as image using the
		/// provided binding. Leave the "texture"
		/// attribute of _binding as zero, since
		/// it will be replaced by this texture
		/// handle
		/// </summary>
		/// <param name="_binding"></param>
		std::optional<GLError> BindImage(ImageBinding _binding) const;

		FORCE_INLINE uint32_t GetTextureHandle() const {
			return m_tex_handle;
		}

		FORCE_INLINE uint32_t GetMipLevels() const {
			return m_mipmap_levels;
		}

		FORCE_INLINE uint32_t GetWidth() const {
			return m_width;
		}

		FORCE_INLINE uint32_t GetHeight() const {
			return m_height;
		}

		FORCE_INLINE TextureFormat GetFormat() const {
			return m_format;
		}

		FORCE_INLINE uint32_t GetMipLevelWidht(uint32_t _level) const {
			return std::max<uint32_t>(1, (m_width >> _level));
		}

		FORCE_INLINE uint32_t GetMipLevelHeight(uint32_t _level) const {
			return std::max<uint32_t>(1, (m_height >> _level));
		}

		uint32_t GetPixelSizeBytes() const;

		FORCE_INLINE size_t GetSizeBytes() const {
			return (size_t)m_width * (size_t)m_height * (size_t)GetPixelSizeBytes();
		}

		FORCE_INLINE SamplerFilter GetUpscaleFilter() const {
			return m_mag_filter;
		}

		FORCE_INLINE SamplerFilter GetDownscaleFilter() const {
			return m_min_filter;
		}

	private :
		Texture2D();
		Texture2D(Texture2D const& _other) = delete;

	private :
		/// Handle to OpenGL texture
		uint32_t      m_tex_handle;
		uint32_t      m_mipmap_levels;
		uint32_t      m_width;
		uint32_t      m_height;
		TextureFormat m_format;
		SamplerFilter m_min_filter;
		SamplerFilter m_mag_filter;
	};
}