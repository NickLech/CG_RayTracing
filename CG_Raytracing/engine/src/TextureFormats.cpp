#include <TextureFormats.hpp>
#include <Utility.hpp>

#include <bit>
#include <GL/glew.h>

namespace cg_raytracing {
	std::expected<uint32_t, GLError> GetOpenGLTextureFormatFromEnum(TextureFormat _format) {
		auto has_int_flag = to_bool(_format & TextureFormat::INT_FLAG);
		auto has_uint_flag = to_bool(_format & TextureFormat::UINT_FLAG);
		auto has_snorm_flag = to_bool(_format & TextureFormat::SNORM_FLAG);

		auto flags = (uint32_t)(std::to_underlying(_format) >> 6) & 0x7;

		if(std::popcount(flags) > 1) {
			return std::unexpected{GLError::INVALID_TEXTURE_FORMAT};
		}

		if (has_int_flag) {
			_format = _format ^ TextureFormat::INT_FLAG;
		}
		else if (has_uint_flag) {
			_format = _format ^ TextureFormat::UINT_FLAG;
		} 
		else if (has_snorm_flag) {
			_format = _format ^ TextureFormat::SNORM_FLAG;
		}

		switch (_format)
		{
		case TextureFormat::R8: {
			if (has_int_flag) {
				return GL_R8I;
			}
			else if (has_uint_flag) {
				return GL_R8UI;
			}
			else if (has_snorm_flag) {
				return GL_R8_SNORM;
			}
			return GL_R8;
		} break;
		case TextureFormat::R16: {
			if (has_int_flag) {
				return GL_R16I;
			}
			else if (has_uint_flag) {
				return GL_R16UI;
			}
			else if (has_snorm_flag) {
				return GL_R16_SNORM;
			}
			return GL_R16;
		} break;
		case TextureFormat::R32: {
			if (has_int_flag) {
				return GL_R32I;
			}
			else if (has_uint_flag) {
				return GL_R32UI;
			}
			
			return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
		} break;
		case TextureFormat::RG8: {
			if (has_int_flag) {
				return GL_RG8I;
			}
			else if (has_uint_flag) {
				return GL_RG8UI;
			}
			else if (has_snorm_flag) {
				return GL_RG8_SNORM;
			}
			return GL_RG8;
		} break;
		case TextureFormat::RG16: {
			if (has_int_flag) {
				return GL_RG16I;
			}
			else if (has_uint_flag) {
				return GL_RG16UI;
			}
			else if (has_snorm_flag) {
				return GL_RG16_SNORM;
			}
			return GL_RG16;
		} break;
		case TextureFormat::RGB4: {
			if (has_int_flag) {
				return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
			}
			else if (has_uint_flag) {
				return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
			}
			else if (has_snorm_flag) {
				return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
			}
			return GL_RGB4;
		} break;
		case TextureFormat::RGB5: {
			if (has_int_flag) {
				return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
			}
			else if (has_uint_flag) {
				return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
			}
			else if (has_snorm_flag) {
				return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
			}
			return GL_RGB5;
		} break;
		case TextureFormat::RGB8: {
			if (has_int_flag) {
				return GL_RGB8I;
			}
			else if (has_uint_flag) {
				return GL_RGB8UI;
			}
			else if (has_snorm_flag) {
				return GL_RGB8_SNORM;
			}
			return GL_RGB8;
		} break;
		case TextureFormat::RGBA4: {
			if (has_int_flag) {
				return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
			}
			else if (has_uint_flag) {
				return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
			}
			else if (has_snorm_flag) {
				return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
			}
			return GL_RGBA4;
		} break;
		case TextureFormat::RGB5A1: {
			if (has_int_flag) {
				return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
			}
			else if (has_uint_flag) {
				return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
			}
			else if (has_snorm_flag) {
				return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
			}
			return GL_RGB5_A1;
		} break;
		case TextureFormat::RGBA8: {
			if (has_int_flag) {
				return GL_RGBA8I;
			}
			else if (has_uint_flag) {
				return GL_RGBA8UI;
			}
			else if (has_snorm_flag) {
				return GL_RGBA8_SNORM;
			}
			return GL_RGBA8;
		} break;
		case TextureFormat::DEPTH32: {
			if (has_int_flag) {
				return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
			}
			else if (has_uint_flag) {
				return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
			}
			else if (has_snorm_flag) {
				return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
			}
			return GL_DEPTH_COMPONENT32F;
		} break;
		case TextureFormat::DEPTH24: {
			if (has_int_flag) {
				return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
			}
			else if (has_uint_flag) {
				return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
			}
			else if (has_snorm_flag) {
				return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
			}
			return GL_DEPTH_COMPONENT24;
		} break;
		case TextureFormat::DEPTH16: {
			if (has_int_flag) {
				return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
			}
			else if (has_uint_flag) {
				return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
			}
			else if (has_snorm_flag) {
				return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
			}
			return GL_DEPTH_COMPONENT16;
		} break;
		case TextureFormat::DEPTH32_STENCIL8: {
			if (has_int_flag) {
				return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
			}
			else if (has_uint_flag) {
				return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
			}
			else if (has_snorm_flag) {
				return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
			}
			return GL_DEPTH32F_STENCIL8;
		} break;
		case TextureFormat::DEPTH24_STENCIL8: {
			if (has_int_flag) {
				return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
			}
			else if (has_uint_flag) {
				return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
			}
			else if (has_snorm_flag) {
				return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
			}
			return GL_DEPTH24_STENCIL8;
		} break;
		case TextureFormat::STENCIL8: {
			if (has_int_flag) {
				return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
			}
			else if (has_uint_flag) {
				return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
			}
			else if (has_snorm_flag) {
				return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };;
			}
			return GL_STENCIL_INDEX8;
		} break;
		default:
			return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };
			break;
		}

		common::Unreachable();
		return std::unexpected{ GLError::INVALID_TEXTURE_FORMAT };
	}

	std::expected<uint32_t, GLError> GetOpenGLImageAccessFromEnum(ImageAccessFlags _access) {
		switch (_access)
		{
		case ImageAccessFlags::READ:
			return GL_READ_ONLY;
		case ImageAccessFlags::WRITE:
			return GL_WRITE_ONLY;
		case ImageAccessFlags::READWRITE:
			return GL_READ_WRITE;
		default:
			return std::unexpected{GLError::INVALID_TEXTURE_ACCESS};
		}
		common::Unreachable();
		return std::unexpected{ GLError::INVALID_TEXTURE_ACCESS };
	}

	std::expected<uint32_t, GLError> GetOpenGLPixelFormatFromEnum(PixelFormat _format) {
		switch (_format)
		{
		case PixelFormat::RED:
			return GL_RED;
		case PixelFormat::RG:
			return GL_RG;
		case PixelFormat::RGB:
			return GL_RGB;
		case PixelFormat::BGR:
			return GL_BGR;
		case PixelFormat::RGBA:
			return GL_RGBA;
		case PixelFormat::BGRA:
			return GL_BGRA;
		case PixelFormat::DEPTH:
			return GL_DEPTH_COMPONENT;
		case PixelFormat::STENCIL:
			return GL_STENCIL_INDEX;
		default:
			return std::unexpected{ GLError::INVALID_PIXEL_FORMAT };
			break;
		}

		common::Unreachable();
		return std::unexpected{ GLError::INVALID_PIXEL_FORMAT };
	}

	std::expected<uint32_t, GLError> GetOpenGLDatatypeFromEnum(PixelDataType _type) {
		switch (_type)
		{
		case PixelDataType::UNSIGNED_BYTE:
			return GL_UNSIGNED_BYTE;
		case PixelDataType::BYTE:
			return GL_BYTE;
		case PixelDataType::USHOT:
			return GL_UNSIGNED_SHORT;
		case PixelDataType::SHORT:
			return GL_SHORT;
		case PixelDataType::UINT:
			return GL_UNSIGNED_INT;
		case PixelDataType::INT:
			return GL_INT;
		case PixelDataType::FLOAT:
			return GL_FLOAT;
		case PixelDataType::UINT8_REV:
			return GL_UNSIGNED_INT_8_8_8_8_REV;
		case PixelDataType::USHORT4:
			return GL_UNSIGNED_SHORT_4_4_4_4;
		case PixelDataType::USHORT4_REV:
			return GL_UNSIGNED_SHORT_4_4_4_4_REV;
		case PixelDataType::USHORT5551:
			return GL_UNSIGNED_SHORT_5_5_5_1;
		case PixelDataType::USHORT1555_REV:
			return GL_UNSIGNED_SHORT_1_5_5_5_REV;
		default:
			return std::unexpected{ GLError::INVALID_PIXEL_DATATYPE };
			break;
		}

		common::Unreachable();
		return std::unexpected{ GLError::INVALID_PIXEL_DATATYPE };
	}

	std::expected<uint32_t, GLError> GetPixelSizeBytesFromEnum(TextureFormat _format) {
		switch (_format)
		{
		case TextureFormat::R8:
			return 1;
		case TextureFormat::R16:
			return 2;
		case TextureFormat::R32:
			return 4;
		case TextureFormat::RG8:
			return 2;
		case TextureFormat::RG16:
			return 4;
		case TextureFormat::RGB4:
			return 2;
		case TextureFormat::RGB5:
			return 2;
		case TextureFormat::RGB8:
			return 3;
		case TextureFormat::RGBA4:
			return 2;
		case TextureFormat::RGB5A1:
			return 2;
		case TextureFormat::RGBA8:
			return 4;
		case TextureFormat::DEPTH32:
			return 4;
		case TextureFormat::DEPTH24:
			return 3;
		case TextureFormat::DEPTH16:
			return 2;
		case TextureFormat::DEPTH32_STENCIL8:
			return 5;
		case TextureFormat::DEPTH24_STENCIL8:
			return 4;
		case TextureFormat::STENCIL8:
			return 1;
		default:
			break;
		}

		return std::unexpected{GLError::INVALID_TEXTURE_FORMAT};
	}
}

