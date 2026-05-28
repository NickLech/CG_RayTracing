#pragma once

#include "stb_image.h"
#include "vec3.hpp"
#include <filesystem>

namespace cg_raytracing::geometry {
class Texture {
  public:
    Texture(const std::filesystem::path _path);
    ~Texture();

    math::Vec3 GetPixel(float _u, float _v);

  private:
    int m_width;
    int m_height;
    int m_channels;
    unsigned char* m_data;
};
} // namespace cg_raytracing::geometry
