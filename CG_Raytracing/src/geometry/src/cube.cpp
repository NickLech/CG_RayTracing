#include "cube.hpp"

namespace cg_raytracing::geometry {

    Cube::Cube(math::Vec3 _center, float _half_size,
           std::shared_ptr<Material> _material) :
    m_material(_material),
    m_triangles(BuildTriangles(_center, _half_size)),
    m_center{_center}, m_size{_half_size * 2.f} {}
    
    std::optional<HitRecord> Cube::Hit(const math::Ray& _ray,
                                       float _t_min,
                                       float _t_max) const {
        std::optional<HitRecord> closest;
        float closest_t = _t_max;
    
        for (const auto& tri : m_triangles) {
            auto hit = tri.Hit(_ray, _t_min, closest_t);
            if (hit) {
                closest_t = hit->m_t;
                closest   = hit;
            }
        }
    
        return closest;
    }
    
    BoundingBox Cube::GetBoundingBox() const {
        return BoundingBox::Create(m_center, m_size, m_size, m_size);
    }
    
    std::array<Triangle, 12> Cube::BuildTriangles(math::Vec3 _center,
                                                   float     _half_size) {
        const float s  = _half_size;
        const float cx = _center.x, cy = _center.y, cz = _center.z;
    
        const math::Vec3 v000(cx-s, cy-s, cz-s);
        const math::Vec3 v100(cx+s, cy-s, cz-s);
        const math::Vec3 v010(cx-s, cy+s, cz-s);
        const math::Vec3 v110(cx+s, cy+s, cz-s);
        const math::Vec3 v001(cx-s, cy-s, cz+s);
        const math::Vec3 v101(cx+s, cy-s, cz+s);
        const math::Vec3 v011(cx-s, cy+s, cz+s);
        const math::Vec3 v111(cx+s, cy+s, cz+s);
    
        const Material* mat = m_material.get();
    
        return {{
            // Front face  (z+)
            { v001, v101, v111, mat },
            { v001, v111, v011, mat },
            // Back face   (z-)
            { v100, v000, v010, mat },
            { v100, v010, v110, mat },
            // Left face   (x-)
            { v000, v001, v011, mat },
            { v000, v011, v010, mat },
            // Right face  (x+)
            { v101, v100, v110, mat },
            { v101, v110, v111, mat },
            // Top face    (y+)
            { v010, v011, v111, mat },
            { v010, v111, v110, mat },
            // Bottom face (y-)
            { v000, v100, v101, mat },
            { v000, v101, v001, mat },
        }};
    }

} // namespace cg_raytracing::geometry