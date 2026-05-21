#include "material.hpp"
#include <algorithm>
#include <cmath>
#include <random>

namespace cg_raytracing::geometry {

cg_raytracing::math::Vec3 StandardMaterial::Shade(
    const HitRecord&              _hit,
    const cg_raytracing::math::Vec3& _light_pos,
    const cg_raytracing::math::Vec3& _light_color,
    float                         _light_intensity,
    const cg_raytracing::math::Ray&  _ray) const {

    cg_raytracing::math::Vec3 light_dir = (_light_pos - _hit.m_point).normalized();
    float diffuse = std::max(_hit.m_normal.dot(light_dir), 0.0f);

    cg_raytracing::math::Vec3 color{};
    switch (m_illum) {
        case 0:
            color = m_kd;
            break;
        case 2: {
            cg_raytracing::math::Vec3 view_dir    = (_ray.m_origin - _hit.m_point).normalized();
            cg_raytracing::math::Vec3 reflect_dir = (_hit.m_normal * 2.0f * _hit.m_normal.dot(light_dir) - light_dir).normalized();
            float spec = std::pow(std::max(view_dir.dot(reflect_dir), 0.0f), m_ns);
            color = (m_ka + m_kd * diffuse + m_ks * spec) * _light_color * _light_intensity;
            break;
        }
        case 1:
        default:
            color = (m_ka + m_kd * diffuse) * _light_color * _light_intensity;
            break;
    }
    return color;
}

static math::Vec3 RandomInHemisphere(const math::Vec3& _normal) {
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    math::Vec3 random_dir;
    do {
        random_dir = math::Vec3(dist(rng), dist(rng), dist(rng));
    } while (random_dir.length_squared() > 1.0f);

    if (random_dir.dot(_normal) < 0.0f)
        random_dir = random_dir * -1.0f;

    return random_dir.normalized();
}

std::vector<math::Ray> StandardMaterial::Scatter(
    const math::Ray& _ray_in,
    const HitRecord& _hit,
    int              _num_samples) const {

    std::vector<math::Ray> scattered;
    scattered.reserve(_num_samples);

    for (int i = 0; i < _num_samples; i++) {
        math::Vec3 scatter_dir = RandomInHemisphere(_hit.m_normal);
        scattered.emplace_back(_hit.m_point, scatter_dir);
    }

    return scattered;
}

} // namespace cg_raytracing::geometry