#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "hit_record.hpp"
#include "ray.hpp"
#include "texture.hpp"
#include "vec3.hpp"

namespace cg_raytracing::geometry {

namespace scene_fwd {
struct PointLight;
}

struct Material {

    // Returns true if this material emits light rather than scattering it.
    // Used by the path tracer to accumulate emission instead of absorbing.
    virtual bool IsEmissive() const {
        return false;
    }

    virtual math::Vec3 Shade(const HitRecord &_hit,
                             const math::Ray &_ray) const = 0;

    virtual ~Material() = default;

    virtual std::optional<std::pair<math::Ray, math::Vec3>>
    Scatter(const math::Ray &_ray_in, const HitRecord &_hit) const = 0;
};

// Standard implementation in MTL format
struct StandardMaterial : public Material {
    math::Vec3 m_ka = {0.1f, 0.1f, 0.1f};
    math::Vec3 m_kd = {0.0f, 0.0f, 0.0f};
    math::Vec3 m_ks = {0.5f, 0.5f, 0.5f};
    math::Vec3 m_ke = {0.0f, 0.0f, 0.0f};
    float m_ns = 0.0f;
    float m_ni = 1.0f;
    float m_d = 1.0f;
    int m_illum = 1;

    std::shared_ptr<cg_raytracing::geometry::Texture> m_diffuse_map = nullptr;

    math::Vec3 Shade(const HitRecord &_hit,
                     const math::Ray &_ray) const override;

    std::optional<std::pair<math::Ray, math::Vec3>>
    Scatter(const math::Ray &_ray_in, const HitRecord &_hit) const override;

    bool IsEmissive() const override {
        if ((m_ke.x > 0.0) || (m_ke.y > 0.0) || (m_ke.z > 0.0)) {
            return true;
        } else {
            return false;
        }
    }

    static StandardMaterial Diffuse(math::Vec3 _kd,
                                    math::Vec3 _ka = {0.1f, 0.1f, 0.1f}) {
        StandardMaterial m{};
        m.m_kd = _kd;
        m.m_ka = _ka;
        m.m_illum = 1;
        return m;
    }

    static StandardMaterial Metal(math::Vec3 _kd,
                                  math::Vec3 _ks = {0.9f, 0.9f, 0.9f},
                                  float _ns = 200.0f) {
        StandardMaterial m{};
        m.m_kd = _kd;
        m.m_ks = _ks;
        m.m_ns = _ns;
        m.m_illum = 2;
        return m;
    }
};

struct DielectricMaterial : public Material {
    // Index of refraction (air=1.0, glass≈1.5, water≈1.33, diamond≈2.4)
    float m_ior = 1.5f;
    math::Vec3 m_tint = {1.0f, 1.0f, 1.0f}; // slight color, e.g. {0.8f, 1.0f, 0.8f} for green glass
    float m_reflectivity = 0.0f; // 0 = pure refraction, 1 = pure reflection

    bool IsEmissive() const override {
        return false;
    }

    math::Vec3 Shade(const HitRecord &_hit,
                     const math::Ray &_ray) const override {
        // Dielectrics pass light through — no direct shading contribution
        return {1.0f, 1.0f, 1.0f};
    }

    std::optional<std::pair<math::Ray, math::Vec3>>
    Scatter(const math::Ray &_ray_in, const HitRecord &_hit) const override;

    static DielectricMaterial Create(float _ior = 1.5f, math::Vec3 _tint = {1.0f, 1.0f, 1.0f}, float _reflectivity = 0.0f) {
        DielectricMaterial m{};
        m.m_ior = _ior;
        m.m_tint = _tint;
        m.m_reflectivity = _reflectivity;
        return m;
    }

    static DielectricMaterial Create(float _ior = 1.5f) {
        DielectricMaterial m{};
        m.m_ior = _ior;
        return m;
    }
};

struct EmissiveMaterial : public Material {
    math::Vec3 m_emission; // color and intensity of the emitted light
    float m_intensity = 1.0f;

    bool IsEmissive() const override {
        return true;
    }

    math::Vec3 Shade(const HitRecord &_hit,
                     const math::Ray &_ray) const override {
        // an emissive material simply returns its emission color multiplied by
        // its intensity
        return m_emission * m_intensity;
    }

    std::optional<std::pair<math::Ray, math::Vec3>>
    Scatter(const math::Ray &_ray_in, const HitRecord &_hit) const override {
        // an emissive material does not scatter rays
        return std::nullopt;
    }

    static EmissiveMaterial Create(math::Vec3 _color, float _intensity = 1.0f) {
        EmissiveMaterial m{};
        m.m_emission = _color;
        m.m_intensity = _intensity;
        return m;
    }
};

} // namespace cg_raytracing::geometry
