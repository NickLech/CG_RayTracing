#pragma once

#include "hit_record.hpp"
#include "hittable.hpp"
#include "material.hpp"
#include "ray.hpp"
#include "triangle.hpp"
#include "vec3.hpp"
#include <array>
#include <expected>
#include <filesystem>
#include <fstream>
#include <hittable.hpp>
#include <iostream>
#include <ranges>

#include <optional>
#include <vector>

namespace cg_raytracing::geometry {
class Mesh : public Hittable {
  public:
    std::shared_ptr<Material> m_material;
    std::vector<math::Vec3> m_vertex_positions;
    std::vector<math::Vec3> m_vertex_normals;
    std::vector<std::array<float, 2>> m_face_uv;
    std::vector<std::array<size_t, 3>> m_indices;

    math::Vec3 m_center;

    bool m_smooth_shading;

    Mesh(cg_raytracing::math::Vec3 _center, std::shared_ptr<Material> _material);

    std::optional<HitRecord> Hit(const cg_raytracing::math::Ray &_ray,
                                 float _t_min = TMIN,
                                 float _t_max = TMAX) const override;

    BoundingBox GetBoundingBox() const override;

    std::expected<int, std::string>
    LoadFromObj(std::filesystem::path _obj_path, float _scale);
};

} // namespace cg_raytracing::geometry
