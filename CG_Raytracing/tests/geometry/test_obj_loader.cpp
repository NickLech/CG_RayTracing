#include "mesh.hpp"
#include "material.hpp"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>

TEST_CASE("Mesh OBJ Loader") {
    using namespace cg_raytracing::geometry;
    using namespace cg_raytracing::math;

    std::filesystem::path test_file_path = "./assets/test_obj_file.obj";

    SECTION("File Not Found Boundary") {
        auto dummy_material = std::make_shared<StandardMaterial>(
            StandardMaterial::Diffuse({0.5f, 0.5f, 0.5f})
        );
        Mesh mesh(Vec3(0, 0, 0));
        auto result = mesh.LoadFromObj("./invalid.obj", 1.0);

        REQUIRE(result.has_value() == false);
        REQUIRE(result.error() == "Failed to open file: ./invalid.obj");
    }

    SECTION("Valid Single Face Geometry Integrity") {

        auto dummy_material = std::make_shared<StandardMaterial>(
            StandardMaterial::Diffuse({0.5f, 0.5f, 0.5f})
        );
        Mesh mesh(Vec3(0, 0, 0));
        auto result = mesh.LoadFromObj(test_file_path, 1.0);
        REQUIRE(result == 0);

        // 1. Validate progressive Vertex assembly
        REQUIRE(mesh.m_vertex_positions.size() == 3);
        REQUIRE(mesh.m_vertex_positions[0].x == Catch::Approx(1.5));
        REQUIRE(mesh.m_vertex_positions[0].y == Catch::Approx(2.5));
        REQUIRE(mesh.m_vertex_positions[0].z == Catch::Approx(3.5));
        REQUIRE(mesh.m_vertex_positions[1].x == Catch::Approx(1.5));
        REQUIRE(mesh.m_vertex_positions[1].y == Catch::Approx(2.5));
        REQUIRE(mesh.m_vertex_positions[1].z == Catch::Approx(0.0));
        REQUIRE(mesh.m_vertex_positions[2].x == Catch::Approx(1.5));
        REQUIRE(mesh.m_vertex_positions[2].y == Catch::Approx(0.0));
        REQUIRE(mesh.m_vertex_positions[2].z == Catch::Approx(3.5));

        // 2. Validate progressive Normal assembly
        REQUIRE(mesh.m_vertex_normals.size() == 1);
        REQUIRE(mesh.m_vertex_normals[0].x == Catch::Approx(0.0));
        REQUIRE(mesh.m_vertex_normals[0].y == Catch::Approx(0.0));
        REQUIRE(mesh.m_vertex_normals[0].z == Catch::Approx(1.0));

        // 3. Validate progressive UV array assembly
        REQUIRE(mesh.m_face_uv.size() == 1);
        REQUIRE(mesh.m_face_uv[0][0] == Catch::Approx(0.75f));
        REQUIRE(mesh.m_face_uv[0][1] == Catch::Approx(0.25f));

        // 4. Validate Smooth Shading State Change
        REQUIRE(mesh.m_smooth_shading == true);
    }

    SECTION("Face Creation Index Mapping Verification") {
        // Exact test scenario from your input
        auto dummy_material = std::make_shared<StandardMaterial>(
            StandardMaterial::Diffuse({0.5f, 0.5f, 0.5f})
        );
        Mesh mesh(Vec3(0, 0, 0));
        auto result = mesh.LoadFromObj(test_file_path, 1.0);

        REQUIRE(result.has_value());

        REQUIRE(mesh.m_indices.size() == 3);

        REQUIRE(mesh.m_indices[0][0] == 1);
        REQUIRE(mesh.m_indices[0][1] == 1);
        REQUIRE(mesh.m_indices[0][2] == 1);

        REQUIRE(mesh.m_indices[1][0] == 2);
        REQUIRE(mesh.m_indices[1][1] == 1);
        REQUIRE(mesh.m_indices[1][2] == 1);

        REQUIRE(mesh.m_indices[2][0] == 3);
        REQUIRE(mesh.m_indices[2][1] == 1);
        REQUIRE(mesh.m_indices[2][2] == 1);
    }
}
