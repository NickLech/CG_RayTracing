#include "mesh.hpp"
#include "vec3.hpp"
#include <cinttypes>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
// #include <unistd.h> Exists only on POSIX/UNIX

namespace cg_raytracing::geometry {

Mesh::Mesh(cg_raytracing::math::Vec3 _center) {
    this->m_center = _center;
}
std::optional<HitRecord> Mesh::Hit(const cg_raytracing::math::Ray &_ray,
                                   float _t_min, float _t_max) const {
    using Triangle = cg_raytracing::geometry::Triangle;
    std::optional<HitRecord> closest_hit;
    float closest_hit_distance = _t_max;
    uint32_t triangle_index = 0;

    for (auto triangle : this->m_indices | std::views::chunk(3)) {

        Triangle current_triangle(
            m_vertex_positions[triangle[0][0] - 1] + this->m_center,
            m_vertex_positions[triangle[1][0] - 1] + this->m_center,
            m_vertex_positions[triangle[2][0] - 1] + this->m_center,
            this->m_face_material_map.at(triangle_index).get());

        auto hit_result =
            current_triangle.Hit(_ray, _t_min, closest_hit_distance);
        if (hit_result) {
            closest_hit = hit_result;
            closest_hit_distance = hit_result->m_t;
        }
        triangle_index++;
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
        std::stringstream ss(line);
        std::string command;
        std::string material_name;

        if (!(ss >> command))
            continue;

        if (command == "v") {
            math::Vec3 new_vec;
            ss >> new_vec.x >> new_vec.y >> new_vec.z;
            this->m_vertex_normals.push_back(new_vec);
        }
        if (command == "vn") {
            math::Vec3 new_vec;
            ss >> new_vec.x >> new_vec.y >> new_vec.z;
            new_vec *= _scale;
            this->m_vertex_positions.push_back(new_vec);
        }
        if (command == "vt") {
            std::array<float, 2> new_normal_map;
            ss >> new_normal_map[0] >> new_normal_map[1];
            this->m_face_uv.push_back(new_normal_map);
        }
        if (command == "s") {
            uint8_t value;
            ss >> value;
            if (value == 0) {
                this->m_smooth_shading = true;
            } else {
                this->m_smooth_shading = false;
            }
        }
        if (command == "f") {

            for (int i = 0; i < 3; i++) {
                std::array<size_t, 3> new_face;
                std::string vertex_info;
                ss >> vertex_info;

                for (auto vertex_index_view :
                     vertex_info | std::views::split('/')) {
                    std::string vertex_index_str{
                        std::string_view(vertex_index_view)};
                    if (vertex_index_str.empty()) {
                        vertex_index_str = "0";
                    }
                    new_face[i] = std::stoul(vertex_index_str);
                }
                this->m_indices.push_back(new_face);
            }
        }
        if (command == "mttlib") {
            std::string material_file_string;
            ss >> material_file_string;
            std::filesystem::path material_file_path =
                _obj_path.parent_path() / material_file_string;

            ReadMaterialFromMtl(material_file_path);
        }
        if (command == "usemtl") {
            ss >> material_name;
            if (this->m_material_map.find(material_name) !=
                this->m_material_map.end()) {
                this->m_face_material_map[this->m_indices.size() / 3 - 1] =
                    this->m_material_map[material_name];
            }
        }
    }
    return 0;
}
void Mesh::ReadMaterialFromMtl(std::string _mtl_path) {
    std::ifstream mtl_file(_mtl_path);
    if (!mtl_file.is_open())
        return;

    std::string line;
    while (std::getline(mtl_file, line)) {
        std::stringstream ss(line);
        std::string command;

        if (!(ss >> command))
            continue;

        if (command.empty() || command[0] == '#')
            continue;

        if (command == "newmtl") {
            std::string material_name;
            if (ss >> material_name) {
                auto new_material = std::make_shared<StandardMaterial>();

                this->m_material.push_back(new_material);

                this->m_material_map[material_name] = new_material;
            }
        } else if (command == "Ns") {
            if (!this->m_material.empty()) {
                ss >> this->m_material.back()->m_ns;
            }
        } else if (command == "Ni") {
            if (!this->m_material.empty()) {
                ss >> this->m_material.back()->m_ni;
            }
        } else if (command == "d") {
            if (!this->m_material.empty()) {
                ss >> this->m_material.back()->m_d;
            }
        } else if (command == "illum") {
            if (!this->m_material.empty()) {
                ss >> this->m_material.back()->m_illum;
            }
        } else if (command == "Ka") {
            if (!this->m_material.empty()) {
                auto &ka = this->m_material.back()->m_ka;
                ss >> ka.x >> ka.y >> ka.z;
            }
        } else if (command == "Kd") {
            if (!this->m_material.empty()) {
                auto &kd = this->m_material.back()->m_kd;
                ss >> kd.x >> kd.y >> kd.z;
            }
        } else if (command == "Ks") {
            if (!this->m_material.empty()) {
                auto &ks = this->m_material.back()->m_ks;
                ss >> ks.x >> ks.y >> ks.z;
            }
        }
    }
}

} // namespace cg_raytracing::geometry
