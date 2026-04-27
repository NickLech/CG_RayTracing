#pragma once 

#include <GLError.hpp>

#include <expected>

namespace cg_raytracing {
	/// <summary>
	/// Upscaling/downscaling filters
	/// </summary>
	enum class SamplerFilter {
		NEAREST,
		LINEAR,
		NEAREST_MIPMAP_NEAREST,
		NEAREST_MIPMAP_LINEAR,
		LINEAR_MIPMAP_NEAREST,
		LINEAR_MIPMAP_LINEAR
	};

	std::expected<uint32_t, GLError> GetOpenGLFilteringFromEnum(SamplerFilter _filter);
}