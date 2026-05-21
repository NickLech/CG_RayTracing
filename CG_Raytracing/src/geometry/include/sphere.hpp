#pragma once

#include "material.hpp"
#include "hit_record.hpp"
#include "ray.hpp"
#include "vec3.hpp"
#include <hittable.hpp>
#include <memory>
#include <optional>

namespace cg_raytracing::geometry {

class Sphere : public Hittable {
public:
    cg_raytracing::math::Vec3     m_center;
    float                         m_radius;
    std::shared_ptr<Material>     m_material;

    Sphere(cg_raytracing::math::Vec3 _center, float _radius,
           std::shared_ptr<Material> _material);

    std::optional<HitRecord> Hit(const cg_raytracing::math::Ray& _ray,
                                 float _t_min = TMIN,
                                 float _t_max = TMAX) const override;

    BoundingBox GetBoundingBox() const override;
};

} // namespace cg_raytracing::geometry