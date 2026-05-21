#pragma once

//#include "material.hpp" // Forward declaration to avoid circular dependency with material.hpp
#include "ray.hpp"
#include "vec3.hpp"

namespace cg_raytracing::geometry {

struct Material; // Forward declaration to avoid circular dependency with material.hpp

struct HitRecord {
    cg_raytracing::math::Vec3 m_point;
    cg_raytracing::math::Vec3 m_normal;
    const Material*           m_material;
    float                     m_t;
    bool                      m_front_face;

    void SetFaceNormal(const cg_raytracing::math::Ray&  _ray,
                       const cg_raytracing::math::Vec3& _outward_normal);
};

} // namespace cg_raytracing::geometry