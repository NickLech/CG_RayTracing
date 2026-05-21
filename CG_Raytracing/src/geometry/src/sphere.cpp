#include "material.hpp"
#include <sphere.hpp>

#include <cmath>

namespace cg_raytracing::geometry {

    Sphere::Sphere(cg_raytracing::math::Vec3 _center, float _radius, std::shared_ptr<Material> _material)
    : m_center(_center), m_radius(_radius), m_material(_material) {}
    
    std::optional<HitRecord> Sphere::Hit(const cg_raytracing::math::Ray& _ray,
                                         float _t_min,
                                         float _t_max) const {
        const auto oc = _ray.m_origin - m_center;
    
        const float a  = _ray.m_direction.dot(_ray.m_direction);
        const float hb = oc.dot(_ray.m_direction);
        const float c  = oc.dot(oc) - m_radius * m_radius;
    
        const float discriminant = hb * hb - a * c;
        if (discriminant < 0.0f)
            return std::nullopt;
    
        const float sqrtd = std::sqrt(discriminant);
    
        float root = (-hb - sqrtd) / a;
        if (root <= _t_min || root >= _t_max) {
            root = (-hb + sqrtd) / a;
            if (root <= _t_min || root >= _t_max)
                return std::nullopt;
        }
    
        HitRecord rec{};
        rec.m_t        = root;
        rec.m_point    = _ray.At(root);
        rec.m_material = m_material.get();
        rec.SetFaceNormal(_ray, (rec.m_point - m_center) * (1.0f / m_radius));
    
        return rec;
    }
    
    BoundingBox Sphere::GetBoundingBox() const {
        return BoundingBox::Create(m_center, m_radius*2.f, m_radius*2.f, m_radius*2.f);
    }

} // namespace cg_raytracing::geometry