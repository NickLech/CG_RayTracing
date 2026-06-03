#include "material.hpp"
#include <algorithm>
#include <cmath>
#include <random>
#include <random_prob.hpp>

namespace cg_raytracing::geometry {

cg_raytracing::math::Vec3 StandardMaterial::Shade(
    const HitRecord &_hit, const cg_raytracing::math::Vec3 &_light_pos,
    const cg_raytracing::math::Vec3 &_light_color, float _light_intensity,
    const cg_raytracing::math::Ray &_ray) const {

    cg_raytracing::math::Vec3 active_diffuse = m_kd;
    if (m_diffuse_map) {
        active_diffuse = m_diffuse_map->GetPixel(_hit.m_tex_u, _hit.m_tex_v);
    }

    cg_raytracing::math::Vec3 light_vec = _light_pos - _hit.m_point;
    float dist_sq = light_vec.length_squared();
    cg_raytracing::math::Vec3 light_dir = light_vec.normalized();

    float attenuation = _light_intensity / (dist_sq + 1.0f);
    float diff_factor = std::max<float>(_hit.m_normal.dot(light_dir), 0.0f);

    cg_raytracing::math::Vec3 lighting_result{};

    switch (m_illum) {
    case 0:
        lighting_result = active_diffuse;
        break;
    case 1:
    case 2:
        lighting_result =
            (m_ka + active_diffuse * diff_factor) * _light_color * attenuation;
        break;
    default:
        lighting_result =
            (m_ka + active_diffuse * diff_factor) * _light_color * attenuation;
        break;
    }

    return lighting_result + m_ke;
}

/// Scatters an incoming ray based on the material type.
/// Diffuse: random direction in hemisphere (Lambert).
/// Metal (illum 2): specular reflection with roughness perturbation.
std::optional<std::pair<math::Ray, math::Vec3>>
StandardMaterial::Scatter(const math::Ray &_ray_in,
                          const HitRecord &_hit) const {
    cg_raytracing::math::Vec3 albedo = m_kd;
    if (m_diffuse_map) {
        albedo = m_diffuse_map->GetPixel(_hit.m_tex_u, _hit.m_tex_v);
    }
    // if ke is non-zero, the material is emissive and does not scatter
    if (m_ke.x > 0.0f || m_ke.y > 0.0f || m_ke.z > 0.0f) {
        return std::nullopt;
    }

    // Metal: specular reflection with roughness-based perturbation
    if (m_illum == 2) {
        math::Vec3 reflected =
            _ray_in.m_direction -
            _hit.m_normal * 2.0f * _hit.m_normal.dot(_ray_in.m_direction);
        float fuzz_amount = (m_ns > 0.0f) ? (1.0f / m_ns) : 0.0f;
        math::Vec3 fuzz = RandomInHemisphere(_hit.m_normal) * fuzz_amount;
        return {
            {math::Ray{_hit.m_point, (reflected + fuzz).normalized()}, m_ks}};
    }

    // Diffuse: random hemisphere scatter weighted by albedo
    math::Vec3 scatter_dir = RandomInHemisphere(_hit.m_normal);
    return {{math::Ray{_hit.m_point, scatter_dir}, albedo}};
}

std::optional<std::pair<math::Ray, math::Vec3>>
DielectricMaterial::Scatter(const math::Ray &_ray_in,
                            const HitRecord &_hit) const {
    float refraction_ratio = _hit.m_front_face ? (1.0f / m_ior) : m_ior;

    math::Vec3 unit_dir = _ray_in.m_direction.normalized();

    float cos_theta = (-unit_dir).dot(_hit.m_normal);
    if (cos_theta > 1.0f)
        cos_theta = 1.0f;
    else if (cos_theta < 0.0f)
        cos_theta = 0.0f;

    float sin_theta = std::sqrt(1.0f - cos_theta * cos_theta);

    math::Vec3 refracted{};
    if (refraction_ratio * sin_theta > 1.0f) {
        // Total internal reflection
        refracted =
            unit_dir - _hit.m_normal * 2.0f * unit_dir.dot(_hit.m_normal);
    } else {
        // Snell's law refraction
        math::Vec3 r_perp =
            (unit_dir + _hit.m_normal * cos_theta) * refraction_ratio;
        math::Vec3 r_para =
            _hit.m_normal *
            -std::sqrt(std::abs(1.0f - r_perp.length_squared()));
        refracted = r_perp + r_para;
    }

    // Blend between straight-through (0) and full refraction (1).
    // Lower values make the object appear more transparent with less
    // distortion.
    constexpr float BLEND = 0.01f;
    math::Vec3 direction =
        (unit_dir * (1.0f - BLEND) + refracted * BLEND).normalized();

    return {{math::Ray{_hit.m_point, direction}, {1.0f, 1.0f, 1.0f}}};
}
} // namespace cg_raytracing::geometry
