#include "mesh.hpp"
#include <memory>
#include <cstddef>
#include <optional>
// #include <unistd.h> Exists only on POSIX/UNIX

namespace cg_raytracing::geometry {

Mesh::Mesh(cg_raytracing::math::Vec3 _center, std::shared_ptr<Material> _material) {
    this->m_center   = _center;
    this->m_material = _material;
}
std::optional<HitRecord> Mesh::Hit(const cg_raytracing::math::Ray &_ray,
                                   float _t_min, float _t_max) const {
    using Triangle = cg_raytracing::geometry::Triangle;
    std::optional<HitRecord> closest_hit;
    float closest_hit_distance = _t_max;

    for (auto triangle : this->m_indices | std::views::chunk(3)) {

        Triangle current_triangle(m_vertex_positions[triangle[0][0] - 1] + this->m_center,
                           m_vertex_positions[triangle[1][0] - 1] + this->m_center,
                           m_vertex_positions[triangle[2][0] - 1] + this->m_center,
                           this->m_material.get());

        auto hit_result = current_triangle.Hit(_ray, _t_min, closest_hit_distance);
        if (hit_result) {
            closest_hit = hit_result;
            closest_hit_distance = hit_result->m_t;
        }
    }
    return closest_hit;
};

BoundingBox Mesh::GetBoundingBox() const {
    if (m_vertex_positions.empty()) {
        return BoundingBox();
    }

    float min_x = std::numeric_limits<float>::infinity();
    float min_y = std::numeric_limits<float>::infinity();
    float min_z = std::numeric_limits<float>::infinity();

    float max_x = -std::numeric_limits<float>::infinity();
    float max_y = -std::numeric_limits<float>::infinity();
    float max_z = -std::numeric_limits<float>::infinity();

    for (const auto &vertex : m_vertex_positions) {
        min_x = std::min(min_x, vertex.x);
        min_y = std::min(min_y, vertex.y);
        min_z = std::min(min_z, vertex.z);

        max_x = std::max(max_x, vertex.x);
        max_y = std::max(max_y, vertex.y);
        max_z = std::max(max_z, vertex.z);
    }

    BoundingBox bbox;
    // TODO: implement a method inside the BoundingBox class to create a
    // bounding box from min and max coordinates
    bbox.min_x = min_x + this->m_center.x;
    bbox.max_x = max_x + this->m_center.x;
    bbox.min_y = min_y + this->m_center.y;
    bbox.max_y = max_y + this->m_center.y;
    bbox.min_z = min_z + this->m_center.z;
    bbox.max_z = max_z + this->m_center.z;
    bbox.pos = math::Vec3((min_x + max_x) / 2, (min_y + max_y) / 2,
                          (min_z + max_z) / 2);
    return bbox;
}

std::expected<int, std::string>
Mesh::LoadFromObj(std::filesystem::path _obj_path, float _scale) {
    std::ifstream obj_file(_obj_path);

    if (!obj_file.is_open()) {
        return std::unexpected("Failed to open file: " + _obj_path.string());
    }

    std::string line;

    while (std::getline(obj_file, line)) {
        // 0 -> check what type of input it is
        // 1 -> (v) vertices
        // 2 -> (vn) vertices normals
        // 3 -> (vt) uv coordinates
        // 4 -> (s) smooth shading
        // 5 -> (f) faces v/vt/vn
        uint8_t state = 0;
        uint8_t info_index = 0;

        for (auto part : std::views::split(line, ' ')) {
            std::string pattern{std::string_view(part)};
            switch (state) {
            case 0:
                if (pattern == "v")
                    state = 1;
                if (pattern == "vn")
                    state = 2;
                if (pattern == "vt")
                    state = 3;
                if (pattern == "s")
                    state = 4;
                if (pattern == "f")
                    state = 5;

                break;
            case 1:
                // TODO:append to vertex array
                if (info_index == 0) {
                    this->m_vertex_positions.push_back(
                        math::Vec3(std::stof(pattern) * _scale, 0, 0));
                } else if (info_index == 1) {
                    this->m_vertex_positions.back().y = -std::stof(pattern) * _scale;
                } else {
                    this->m_vertex_positions.back().z = std::stof(pattern) * _scale;
                }
                info_index += 1;
                break;
            case 2:
                // TODO:append to normal array
                if (info_index == 0) {
                    this->m_vertex_normals.push_back(
                        math::Vec3(std::stof(pattern), 0, 0));
                } else if (info_index == 1) {
                    this->m_vertex_normals.back().y = std::stof(pattern);
                } else {
                    this->m_vertex_normals.back().z = std::stof(pattern);
                }
                info_index += 1;
                break;
            case 3:
                // TODO:append to uv coordinate array
                if (info_index == 0) {
                    this->m_face_uv.push_back(
                        std::array<float, 2>{std::stof(pattern), 0});

                } else {
                    this->m_face_uv.back()[1] = std::stof(pattern);
                }
                info_index += 1;
                break;
            case 4: {
                if (pattern == "0")
                    this->m_smooth_shading = false;
                else
                    this->m_smooth_shading = true;
                break;
            }
            case 5:
                size_t count = 0;
                for (auto triangle_index : std::views::split(part, '/')) {
                    std::string string_index{std::string_view(triangle_index)};
                    size_t int_index = std::stoi(string_index);
                    if (count == 0) {
                        this->m_indices.push_back(std::array<size_t, 3>());
                    }

                    this->m_indices.back()[count] = int_index;

                    count += 1;
                }
                break;
            }
        }
    }
    return 0;
}
} // namespace cg_raytracing::geometry
