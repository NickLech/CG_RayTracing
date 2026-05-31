#include "mesh.hpp"
#include "vec3.hpp"
#include <cinttypes>
#include <cstddef>
#include <memory>
#include <optional>
#include <print>
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
            this->m_face_material_vector[triangle_index].get());

        auto hit_result =
            current_triangle.Hit(_ray, _t_min, closest_hit_distance);
        if (hit_result) {
            float b_u = hit_result->m_u; 
            float b_v = hit_result->m_v;
            float b_w = 1.0f - b_u - b_v;

            auto uv0 = m_face_uv[triangle[0][1] - 1];
            auto uv1 = m_face_uv[triangle[1][1] - 1];
            auto uv2 = m_face_uv[triangle[2][1] - 1];

            hit_result->m_tex_u = b_w * uv0[0] + b_u * uv1[0] + b_v * uv2[0];
            hit_result->m_tex_v = b_w * uv0[1] + b_u * uv1[1] + b_v * uv2[1];

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
    uint32_t face_count = 0;
    std::shared_ptr<StandardMaterial> current_material;

    while (std::getline(obj_file, line)) {
        // 0 -> check what type of input it is
        // 1 -> (v) vertices
        // 2 -> (vn) vertices normals
        // 3 -> (vt) uv coordinates
        // 4 -> (s) smooth shading
        // 5 -> (f) faces v/vt/vn
        // 6 -> (mtllib) material library
        // 7 -> (usemtl) use material
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
                if (pattern == "f") {
                    state = 5;
                    this->m_face_material_vector.push_back(current_material);
                }
                if (pattern == "mtllib")
                    state = 6;
                if (pattern == "usemtl")
                    state = 7;

                break;
            case 1:
                // TODO:append to vertex array
                if (info_index == 0) {
                    this->m_vertex_positions.push_back(
                        math::Vec3(std::stof(pattern) * _scale, 0, 0));
                } else if (info_index == 1) {
                    this->m_vertex_positions.back().y =
                        std::stof(pattern) * _scale;
                } else {
                    this->m_vertex_positions.back().z =
                        std::stof(pattern) * _scale;
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
            case 5: {
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
            case 6: {
                std::filesystem::path material_file_path =
                    _obj_path.parent_path() / pattern;

                ReadMaterialFromMtl(material_file_path.string());
                break;
            }
            case 7: {

                if (this->m_material_map.find(pattern) !=
                    this->m_material_map.end()) {
                    current_material = this->m_material_map[pattern];
                }
                break;
            }
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
        } else if (command == "map_Kd") {
            std::string texture_path_str;
            if (ss >> texture_path_str && !this->m_material.empty()) {
                // Resolve the path relative to the MTL file location
                std::filesystem::path mtl_dir =
                    std::filesystem::path(_mtl_path).parent_path().parent_path();
                // print mtl_dir and texture_path_str for debugging
                std::println(std::cout, "MTL Directory: {}", mtl_dir.string());
                std::println(std::cout, "Texture Path: {}", texture_path_str);
                std::string full_texture_path =
                    (mtl_dir / "textures" / texture_path_str).string();

                // Assign the texture to the current material
                this->m_material.back()->m_diffuse_map =
                    std::make_shared<Texture>(full_texture_path);
            }
        }
    }
}
void Mesh::Rotate(const math::Vec3 &_rotation_angles) {
    // TODO: optimize the bounding box calculation because this sucks
    for (auto &vertex : this->m_vertex_positions) {
        vertex.Rotate(_rotation_angles);
    }
}

void Mesh::Translate(const math::Vec3 &_translation_vector) {
    for (auto &vertex : this->m_vertex_positions) {
        vertex += _translation_vector;
    }
}
} // namespace cg_raytracing::geometry
