#import "texture.hpp"

namespace cg_raytracing::geometry {
Texture::Texture(const std::filesystem::path _path) {
    stbi_set_flip_vertically_on_load(true);
    m_data = stbi_load(_path.c_str(), &m_width, &m_height, &m_channels, 3);
}

Texture::~Texture() {
    if (m_data)
        stbi_image_free(m_data);
}

math::Vec3 Texture::GetPixel(float _u, float _v) {
    if (!m_data)
        return {1, 0, 1}; // Magenta error color

    _u -= std::floor(_u);
    _v -= std::floor(_v);

    int x = static_cast<int>(_u * (m_width - 1));
    int y = static_cast<int>(_v * (m_height - 1));

    int idx = (y * m_width + x) * 3;
    return {m_data[idx + 0] / 255.0f, m_data[idx + 1] / 255.0f,
            m_data[idx + 2] / 255.0f};
}

} // namespace cg_raytracing::geometry
