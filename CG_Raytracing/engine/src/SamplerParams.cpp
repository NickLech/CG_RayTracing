#include <SamplerParams.hpp>

#include <GL/glew.h>

namespace cg_raytracing {
	std::expected<uint32_t, GLError> GetOpenGLFilteringFromEnum(SamplerFilter _filter) {
		switch (_filter)
		{
		case SamplerFilter::NEAREST:
			return GL_NEAREST;
		case SamplerFilter::LINEAR:
			return GL_LINEAR;
		case SamplerFilter::NEAREST_MIPMAP_NEAREST:
			return GL_NEAREST_MIPMAP_NEAREST;
		case SamplerFilter::NEAREST_MIPMAP_LINEAR:
			return GL_NEAREST_MIPMAP_LINEAR;
		case SamplerFilter::LINEAR_MIPMAP_NEAREST:
			return GL_LINEAR_MIPMAP_NEAREST;
		case SamplerFilter::LINEAR_MIPMAP_LINEAR:
			return GL_LINEAR_MIPMAP_LINEAR;
		default:
			break;
		}

		return std::unexpected{ GLError::INVALID_SAMPLE_FILTER };
	}
}