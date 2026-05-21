#include "mesh.hpp"
#include "vec3.hpp"
#include "material.hpp"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <filesystem>

TEST_CASE("Mesh Bounding Box Validation Suite",
          "[geometry][mesh][bounding_box]") {
    using namespace cg_raytracing::geometry;
    using namespace cg_raytracing::math;

    std::filesystem::path test_file_path = "./assets/test_obj_file.obj";

    REQUIRE(std::filesystem::exists(test_file_path));

    SECTION("AABB Generation from Physical Mesh File") {
        auto dummy_material = std::make_shared<StandardMaterial>(
            StandardMaterial::Diffuse({0.5f, 0.5f, 0.5f})
        );
        Mesh mesh(Vec3(0.0f, 0.0f, 0.0f), dummy_material);

        auto load_result = mesh.LoadFromObj(test_file_path, 1.0);
        REQUIRE(load_result.has_value());

        BoundingBox bbox = mesh.GetBoundingBox();

        REQUIRE(bbox.min_x == Catch::Approx(1.5f));
        REQUIRE(bbox.max_x == Catch::Approx(1.5f));

        REQUIRE(bbox.min_y == Catch::Approx(0.0f));
        REQUIRE(bbox.max_y == Catch::Approx(2.5f));

        REQUIRE(bbox.min_z == Catch::Approx(0.0f));
        REQUIRE(bbox.max_z == Catch::Approx(3.5f));

        REQUIRE(bbox.SizeX() == Catch::Approx(0.0f));
        REQUIRE(bbox.SizeY() == Catch::Approx(2.5f));
        REQUIRE(bbox.SizeZ() == Catch::Approx(3.5f));

        REQUIRE(bbox.Volume() == Catch::Approx(0.0f));
    }
}
