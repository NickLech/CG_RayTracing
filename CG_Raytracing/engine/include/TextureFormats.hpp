#pragma once

#include <Macros.hpp>
#include <GLError.hpp>

#include <expected>

namespace cg_raytracing {
	/// <summary>
	/// Many possible texture formats. 
	/// Some of them can be ORed together
	/// with the _FLAG formats
	/// </summary>
	enum class TextureFormat {
		R8,
		R16,
		R32,
		RG8,
		RG16,
		RGB4,
		RGB5,
		RGB8,
		RGBA4,
		RGB5A1,
		RGBA8,
		DEPTH32,
		DEPTH24,
		DEPTH16,
		DEPTH32_STENCIL8,
		DEPTH24_STENCIL8,
		STENCIL8,
		INT_FLAG = 1 << 6,   // Underlying data is signed int
		UINT_FLAG = 1 << 7,  // Underlying data is unsigned int
		SNORM_FLAG = 1 << 8  // Data is converted to signed normalized values when accessed
	};

	DECLARE_ENUM_LOGICAL_OPS(TextureFormat);

	std::expected<uint32_t, GLError> GetOpenGLTextureFormatFromEnum(TextureFormat _format);

	std::expected<uint32_t, GLError> GetPixelSizeBytesFromEnum(TextureFormat _format);

	/// <summary>
	/// Image access flags for image bindings
	/// </summary>
	enum class ImageAccessFlags {
		READ,
		WRITE,
		READWRITE
	};

	std::expected<uint32_t, GLError> GetOpenGLImageAccessFromEnum(ImageAccessFlags _access);

	/// <summary>
	/// Format of pixel data when copying from CPU
	/// to texture
	/// </summary>
	enum class PixelFormat {
		RED,
		RG,
		RGB,
		BGR,
		RGBA,
		BGRA,
		DEPTH,
		STENCIL
	};

	std::expected<uint32_t, GLError> GetOpenGLPixelFormatFromEnum(PixelFormat _format);

	/// <summary>
	/// Datatype of pixel data when copying from CPU
	/// to texture
	/// </summary>
	enum class PixelDataType {
		UNSIGNED_BYTE,
		BYTE,
		USHOT,
		SHORT,
		UINT,
		INT,
		FLOAT,
		UINT8_REV,
		USHORT4,
		USHORT4_REV,
		USHORT5551,
		USHORT1555_REV
	};

	std::expected<uint32_t, GLError> GetOpenGLDatatypeFromEnum(PixelDataType _type);
}