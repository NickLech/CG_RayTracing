#include "material.hpp"
#include <algorithm>
#include <cmath>
#include <random>
#include <random_prob.hpp>

namespace cg_raytracing::geometry {

cg_raytracing::math::Vec3 StandardMaterial::Shade(
    const HitRecord&              _hit,
    const cg_raytracing::math::Vec3& _light_pos,
    const cg_raytracing::math::Vec3& _light_color,
    float                         _light_intensity,
    const cg_raytracing::math::Ray&  _ray) const {

    cg_raytracing::math::Vec3 light_dir = (_light_pos - _hit.m_point).normalized();
    float diffuse = std::max<float>(_hit.m_normal.dot(light_dir), 0.0f);

    cg_raytracing::math::Vec3 color{};
    switch (m_illum) {
        case 0:
            color = m_kd;
            break;
        case 2: {
            cg_raytracing::math::Vec3 view_dir    = (_ray.m_origin - _hit.m_point).normalized();
            cg_raytracing::math::Vec3 reflect_dir = (_hit.m_normal * 2.0f * _hit.m_normal.dot(light_dir) - light_dir).normalized();
            float spec = std::pow(std::max<float>(view_dir.dot(reflect_dir), 0.0f), m_ns);
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

/// Generates _num_samples scattered rays from the hit point,
/// each with a random direction in the hemisphere of the surface normal.
std::optional<std::pair<math::Ray, math::Vec3>> StandardMaterial::Scatter(
    const math::Ray& _ray_in,
    const HitRecord& _hit) const {

    math::Vec3 scatter_dir = RandomInHemisphere(_hit.m_normal);

    return { {math::Ray{_hit.m_point, scatter_dir}, m_ka} };
}

} // namespace cg_raytracing::geometry