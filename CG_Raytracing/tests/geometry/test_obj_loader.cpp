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

TEST_CASE("material file loader test") {
    using namespace cg_raytracing::geometry;
    using namespace cg_raytracing::math;

    // Explicitly path tracking the target validation asset
    std::filesystem::path test_mtl_path = "./assets/test_material_file.mtl";

    REQUIRE(std::filesystem::exists(test_mtl_path));

    SECTION("Validate Property Extraction and Map Association") {
        // Instantiate mesh container on origin
        Mesh mesh(Vec3(0.0f, 0.0f, 0.0f));

        // Execute your parser over the filesystem target
        mesh.ReadMaterialFromMtl(test_mtl_path.string());

        // 1. Verify collections allocated the expected count from file headers
        REQUIRE(mesh.m_material.size() == 2);
        REQUIRE(mesh.m_material_map.size() == 2);

        // 2. Test lookup existence inside your string mapping structure
        REQUIRE(mesh.m_material_map.contains("Wood"));
        REQUIRE(mesh.m_material_map.contains("RedPlastic"));

        // ==========================================
        // Verify Target Material 1: "Wood"
        // ==========================================
        auto wood_mat = mesh.m_material_map["Wood"];
        REQUIRE(wood_mat != nullptr);

        // Verify basic uniform floating-point definitions
        REQUIRE(wood_mat->m_ns == Catch::Approx(96.078431f));
        REQUIRE(wood_mat->m_ni == Catch::Approx(1.000000f));
        REQUIRE(wood_mat->m_d  == Catch::Approx(1.000000f));
        REQUIRE(wood_mat->m_illum == 0);

        // Verify vector transformations matching the parsed strings
        REQUIRE(wood_mat->m_ka.x == Catch::Approx(1.000000f));
        REQUIRE(wood_mat->m_ka.y == Catch::Approx(1.000000f));
        REQUIRE(wood_mat->m_ka.z == Catch::Approx(1.000000f));

        REQUIRE(wood_mat->m_kd.x == Catch::Approx(0.640000f));
        REQUIRE(wood_mat->m_kd.y == Catch::Approx(0.640000f));
        REQUIRE(wood_mat->m_kd.z == Catch::Approx(0.640000f));

        REQUIRE(wood_mat->m_ks.x == Catch::Approx(0.500000f));
        REQUIRE(wood_mat->m_ks.y == Catch::Approx(0.500000f));
        REQUIRE(wood_mat->m_ks.z == Catch::Approx(0.500000f));

        // ==========================================
        // Verify Target Material 2: "RedPlastic"
        // ==========================================
        auto plastic_mat = mesh.m_material_map["RedPlastic"];
        REQUIRE(plastic_mat != nullptr);

        // Make sure properties didn't bleed across active allocation markers
        REQUIRE(plastic_mat->m_kd.x == Catch::Approx(1.000000f));
        REQUIRE(plastic_mat->m_kd.y == Catch::Approx(0.000000f));
        REQUIRE(plastic_mat->m_kd.z == Catch::Approx(0.000000f));
        REQUIRE(plastic_mat->m_ns == Catch::Approx(50.0f));
    }
}
TEST_CASE("obj + mtl file loader") {
    using namespace cg_raytracing::geometry;
    using namespace cg_raytracing::math;
    std::filesystem::path test_obj_path = "./assets/Lampada.obj";

    REQUIRE(std::filesystem::exists(test_obj_path));

    SECTION("Lampada"){
        Mesh mesh(Vec3(0, 0, 0));
        auto result = mesh.LoadFromObj(test_obj_path, 1.0);


        REQUIRE(result == 0);
        REQUIRE(mesh.m_vertex_positions.size() == 226);

        REQUIRE(mesh.m_material.size() == 3);
        REQUIRE(mesh.m_material_map.size() == 3);

        REQUIRE(mesh.m_material_map.contains("base"));
        REQUIRE(mesh.m_material_map.contains("cappello"));
        REQUIRE(mesh.m_material_map.contains("lampada"));

        auto wood_mat = mesh.m_material_map["lampada"];
        REQUIRE(wood_mat != nullptr);

        // Verify basic uniform floating-point definitions
        REQUIRE(wood_mat->m_ns == Catch::Approx(250.0f));
        REQUIRE(wood_mat->m_ni == Catch::Approx(1.500000f));
        REQUIRE(wood_mat->m_d  == Catch::Approx(1.000000f));
        REQUIRE(wood_mat->m_illum == 2);

        // Verify vector transformations matching the parsed strings
        REQUIRE(wood_mat->m_ka.x == Catch::Approx(1.000000f));
        REQUIRE(wood_mat->m_ka.y == Catch::Approx(1.000000f));
        REQUIRE(wood_mat->m_ka.z == Catch::Approx(1.000000f));

        REQUIRE(wood_mat->m_kd.x == Catch::Approx(0.800016f));
        REQUIRE(wood_mat->m_kd.y == Catch::Approx(0.787980f));
        REQUIRE(wood_mat->m_kd.z == Catch::Approx(0.141787f));

        REQUIRE(wood_mat->m_ks.x == Catch::Approx(0.500000f));
        REQUIRE(wood_mat->m_ks.y == Catch::Approx(0.500000f));
        REQUIRE(wood_mat->m_ks.z == Catch::Approx(0.500000f));
    }

}
