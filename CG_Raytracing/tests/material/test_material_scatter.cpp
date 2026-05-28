#include <material.hpp>
#include <ray.hpp>
#include <vec3.hpp>
#include <hit_record.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

using Vec3 = cg_raytracing::math::Vec3;
using Ray = cg_raytracing::math::Ray;

using Material = cg_raytracing::geometry::StandardMaterial;
using HitRecord = cg_raytracing::geometry::HitRecord;

TEST_CASE("Material Scatter Tests") {
    SECTION("Scatter returns a ray") {
        auto material = Material::Diffuse({0.5f, 0.5f, 0.5f});
        HitRecord hit{};
        hit.m_point  = Vec3(0.f, 0.f, 0.f);
        hit.m_normal = Vec3(0.f, 1.f, 0.f);
        Ray incoming(Vec3(0.f, 10.f, 0.f), Vec3(0.f, -1.f, 0.f));

        auto result = material.Scatter(incoming, hit);

        REQUIRE(result.has_value());
    }

    SECTION("Scattered ray stays in correct hemisphere") {
        auto material = Material::Diffuse({0.5f, 0.5f, 0.5f});
        HitRecord hit{};
        hit.m_point  = Vec3(0.f, 0.f, 0.f);
        hit.m_normal = Vec3(0.f, 1.f, 0.f);
        Ray incoming(Vec3(0.f, 10.f, 0.f), Vec3(0.f, -1.f, 0.f));

        auto result = material.Scatter(incoming, hit);
        REQUIRE(result.has_value());

        float dot = result->first.m_direction.dot(hit.m_normal);
        REQUIRE(dot >= 0.0f);
    }

    SECTION("Scattered ray is normalized") {
        auto material = Material::Diffuse({0.5f, 0.5f, 0.5f});
        HitRecord hit{};
        hit.m_point  = Vec3(0.f, 0.f, 0.f);
        hit.m_normal = Vec3(0.f, 1.f, 0.f);
        Ray incoming(Vec3(0.f, 10.f, 0.f), Vec3(0.f, -1.f, 0.f));

        auto result = material.Scatter(incoming, hit);
        REQUIRE(result.has_value());

        float len = result->first.m_direction.length();
        REQUIRE(len == Catch::Approx(1.0f).margin(0.001f));
    }

    SECTION("Scattered ray originates from hit point") {
        auto material = Material::Diffuse({0.5f, 0.5f, 0.5f});
        HitRecord hit{};
        hit.m_point  = Vec3(1.f, 2.f, 3.f);
        hit.m_normal = Vec3(0.f, 1.f, 0.f);
        Ray incoming(Vec3(0.f, 10.f, 0.f), Vec3(0.f, -1.f, 0.f));

        auto result = material.Scatter(incoming, hit);
        REQUIRE(result.has_value());

        REQUIRE(result->first.m_origin.x == Catch::Approx(hit.m_point.x));
        REQUIRE(result->first.m_origin.y == Catch::Approx(hit.m_point.y));
        REQUIRE(result->first.m_origin.z == Catch::Approx(hit.m_point.z));
    }
}