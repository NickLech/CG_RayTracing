#pragma once

#include <vector>
#include <memory>
#include <optional>

#include "vec3.hpp"
#include "ray.hpp"
#include "hit_record.hpp"

namespace cg_raytracing::geometry {

namespace scene_fwd { struct PointLight; }

struct Material {
    virtual math::Vec3 Shade(const HitRecord&  _hit,
                              const math::Vec3& _light_pos,
                              const math::Vec3& _light_color,
                              float             _light_intensity,
                              const math::Ray&  _ray) const = 0;
    virtual ~Material() = default;

    virtual std::optional<std::pair<math::Ray, math::Vec3>> Scatter(
        const math::Ray& _ray_in,
        const HitRecord& _hit
    ) const = 0;
};

// Standard implementation in MTL format
struct StandardMaterial : public Material {
    math::Vec3  m_ka    = {0.1f, 0.1f, 0.1f};
    math::Vec3  m_kd;
    math::Vec3  m_ks    = {0.5f, 0.5f, 0.5f};
    float       m_ns    = 0.0f;
    float       m_ni    = 1.0f;
    float       m_d     = 1.0f;
    int         m_illum = 1;

    math::Vec3 Shade(const HitRecord&  _hit,
                     const math::Vec3& _light_pos,
                     const math::Vec3& _light_color,
                     float             _light_intensity,
                     const math::Ray&  _ray) const override;


    std::optional<std::pair<math::Ray, math::Vec3>> Scatter(
        const math::Ray& _ray_in,
        const HitRecord& _hit
    ) const override;

    static StandardMaterial Diffuse(math::Vec3 _kd,
                                     math::Vec3 _ka = {0.1f, 0.1f, 0.1f}) {
        StandardMaterial m{};
        m.m_kd    = _kd;
        m.m_ka    = _ka;
        m.m_illum = 1;
        return m;
    }

    static StandardMaterial Metal(math::Vec3 _kd,
                                   math::Vec3 _ks = {0.9f, 0.9f, 0.9f},
                                   float _ns = 200.0f) {
        StandardMaterial m{};
        m.m_kd    = _kd;
        m.m_ks    = _ks;
        m.m_ns    = _ns;
        m.m_illum = 2;
        return m;
    }
};

} // namespace cg_raytracing::geometry