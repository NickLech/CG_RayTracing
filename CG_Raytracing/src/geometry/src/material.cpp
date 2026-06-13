#include "material.hpp"
#include <cmath>
#include <random_prob.hpp>

namespace cg_raytracing::geometry {

cg_raytracing::math::Vec3
StandardMaterial::Shade(const HitRecord &_hit,
                        const cg_raytracing::math::Ray &_ray) const {
    cg_raytracing::math::Vec3 active_diffuse = m_kd;
    if (m_diffuse_map) {
        active_diffuse = m_diffuse_map->GetPixel(_hit.m_tex_u, _hit.m_tex_v);
    }

    cg_raytracing::math::Vec3 color{};
    switch (m_illum) {
    case 0:
        // Flat color, no lighting
        color = active_diffuse;
        break;
    case 1:
        // Lambert only: ambient + diffuse
        color = (m_ka + active_diffuse);
        break;
    case 2: {
        // Phong: ambient + diffuse + specular
        // cg_raytracing::math::Vec3 view_dir    = (_ray.m_origin -
        // _hit.m_point).normalized(); cg_raytracing::math::Vec3 reflect_dir =
        // (_hit.m_normal * 2.0f * _hit.m_normal.dot(light_dir) -
        // light_dir).normalized(); float spec =
        // std::pow(std::max<float>(view_dir.dot(reflect_dir), 0.0f), m_ns);
        color = (m_ka + active_diffuse + m_ks);
        break;
    }
    default:
        color = (m_ka + active_diffuse);
        break;
    }
    return color;
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

    // Metal: specular reflection with roughness-based perturbation
    if (m_illum == 2) {
        math::Vec3 reflected =
            _ray_in.m_direction -
            _hit.m_normal * 2.0f * _hit.m_normal.dot(_ray_in.m_direction);
        float fuzz_amount = (m_ns > 0.0f) ? (1.0f / m_ns) : 0.0f;
        math::Vec3 fuzz = RandomInHemisphere(_hit.m_normal) * fuzz_amount;
        return {
            {math::Ray{_hit.m_point, (reflected + fuzz).normalized()}, albedo }};
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

    math::Vec3 direction{};
    if (refraction_ratio * sin_theta > 1.0f) {
        // Total internal reflection
        direction =
            unit_dir - _hit.m_normal * 2.0f * unit_dir.dot(_hit.m_normal);
    } else {
        math::Vec3 r_perp =
            (unit_dir + _hit.m_normal * cos_theta) * refraction_ratio;
        math::Vec3 r_para =
            _hit.m_normal *
            -std::sqrt(std::abs(1.0f - r_perp.length_squared()));
        direction = r_perp + r_para;
    }

    // Nudge origin along the refracted/reflected direction to escape the surface,
    // bypassing the camera.cpp normal-based nudge which goes inward for exit rays
    math::Vec3 origin = _hit.m_point + direction.normalized();

    return {{math::Ray{origin, direction.normalized()}, m_tint}};
}
} // namespace cg_raytracing::geometry
