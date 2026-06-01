#include "material.hpp"
#include <algorithm>
#include <cmath>
#include <random>
#include <random_prob.hpp>

namespace cg_raytracing::geometry {

cg_raytracing::math::Vec3 StandardMaterial::Shade(
    const HitRecord &_hit,
    const cg_raytracing::math::Ray &_ray) const {

    cg_raytracing::math::Vec3 active_diffuse = m_kd;
    if (m_diffuse_map) {
        active_diffuse = m_diffuse_map->GetPixel(_hit.m_tex_u, _hit.m_tex_v);
    }

    // cg_raytracing::math::Vec3 light_dir =
    //     (_light_pos - _hit.m_point).normalized();
    // float diffuse = std::max<float>(_hit.m_normal.dot(light_dir), 0.0f);

    cg_raytracing::math::Vec3 color{};
    switch (m_illum) {
    case 0:
        // Flat color, no lighting
        color = active_diffuse;
        break;
    case 1:
        // Lambert only: ambient + diffuse
        color =
            (m_ka + active_diffuse);
        break;
    case 2: {
        // Phong: ambient + diffuse + specular
        // cg_raytracing::math::Vec3 view_dir =
        //     (_ray.m_origin - _hit.m_point).normalized();
        // cg_raytracing::math::Vec3 reflect_dir =
        //     (_hit.m_normal * 2.0f * _hit.m_normal.dot(light_dir) - light_dir)
        //         .normalized();
        // float spec =
        //     std::pow(std::max<float>(view_dir.dot(reflect_dir), 0.0f), m_ns);
        // color = (m_ka + active_diffuse * diffuse + m_ks * spec) * _light_color *
        //         _light_intensity;
        color =
            (m_ka + active_diffuse);
        break;
    }
    default:
        color =
            (m_ka + active_diffuse);
        break;
    }
    return color + m_ke;
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

} // namespace cg_raytracing::geometry
