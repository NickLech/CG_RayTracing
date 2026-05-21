#include "triangle.hpp"

#include <cmath>
#include <algorithm>

namespace cg_raytracing::geometry {

    Triangle::Triangle(cg_raytracing::math::Vec3 _v0,
                       cg_raytracing::math::Vec3 _v1,
                       cg_raytracing::math::Vec3 _v2,
                       const Material*           _material)
        : m_v0(_v0), m_v1(_v1), m_v2(_v2), m_material(_material) {}
    
    std::optional<HitRecord> Triangle::Hit(const cg_raytracing::math::Ray& _ray,
                                            float _t_min,
                                            float _t_max) const {
        const auto  edge1 = m_v1 - m_v0;
        const auto  edge2 = m_v2 - m_v0;
    
        const auto  h     = edge2.CrossProduct(_ray.m_direction);
        const float a     = edge1.dot(h);
    
        if (std::abs(a) < 1e-8f)
            return std::nullopt;
    
        const float inv_a = 1.0f / a;
        const auto  s     = _ray.m_origin - m_v0;
        const float u     = inv_a * s.dot(h);
    
        if (u < 0.0f || u > 1.0f)
            return std::nullopt;
    
        const auto  q = edge1.CrossProduct(s);
        const float v = inv_a * _ray.m_direction.dot(q);
    
        if (v < 0.0f || u + v > 1.0f)
            return std::nullopt;
    
        const float t = inv_a * edge2.dot(q);
    
        if (t < _t_min || t > _t_max)
            return std::nullopt;
    
        const auto outward_normal = edge1.CrossProduct(edge2).normalized();
    
        HitRecord rec{};
        rec.m_t        = t;
        rec.m_point    = _ray.At(t);
        rec.m_material = m_material;
        rec.SetFaceNormal(_ray, outward_normal);
    
        return rec;
    }
    
    BoundingBox Triangle::GetBoundingBox() const {
        auto min_x = std::min({ m_v0.x, m_v1.x, m_v2.x });
        auto max_x = std::max({ m_v0.x, m_v1.x, m_v2.x });

        auto min_y = std::min({ m_v0.y, m_v1.y, m_v2.y });
        auto max_y = std::max({ m_v0.y, m_v1.y, m_v2.y });

        auto min_z = std::min({ m_v0.z, m_v1.z, m_v2.z });
        auto max_z = std::max({ m_v0.z, m_v1.z, m_v2.z });

        auto pos_x = (min_x + max_x) / 2.f;
        auto pos_y = (min_y + max_y) / 2.f;
        auto pos_z = (min_z + max_z) / 2.f;

        BoundingBox bbox{};
        bbox.min_x = min_x;
        bbox.max_x = max_x;
        bbox.min_y = min_y;
        bbox.max_y = max_y;
        bbox.min_z = min_z;
        bbox.max_z = max_z;
        bbox.pos = math::Vec3(pos_x, pos_y, pos_z);

        if (0 == bbox.Volume()) {
            bbox.max_x += 0.1f;
        }

        return bbox;
    }

} // namespace cg_raytracing::geometry
