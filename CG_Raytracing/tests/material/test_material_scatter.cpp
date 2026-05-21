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

    SECTION("Scatter returns correct number of rays") {

        auto material = Material::Diffuse({0.5f, 0.5f, 0.5f});

        HitRecord hit{};
        hit.m_point  = Vec3(0.f, 0.f, 0.f);
        hit.m_normal = Vec3(0.f, 1.f, 0.f);

        Ray incoming(
            Vec3(0.f, 10.f, 0.f),
            Vec3(0.f, -1.f, 0.f)
        );

        constexpr int NUM_SAMPLES = 16;

        auto scattered = material.Scatter(
            incoming,
            hit,
            NUM_SAMPLES
        );

        REQUIRE(scattered.size() == NUM_SAMPLES);
    }

    SECTION("Scattered rays stay in correct hemisphere") {

        auto material = Material::Diffuse({0.5f, 0.5f, 0.5f});

        HitRecord hit{};
        hit.m_point  = Vec3(0.f, 0.f, 0.f);
        hit.m_normal = Vec3(0.f, 1.f, 0.f);

        Ray incoming(
            Vec3(0.f, 10.f, 0.f),
            Vec3(0.f, -1.f, 0.f)
        );

        auto scattered = material.Scatter(
            incoming,
            hit,
            128
        );

        for (const auto& ray : scattered) {

            float dot = ray.m_direction.dot(hit.m_normal);

            REQUIRE(dot >= 0.0f);
        }
    }

    SECTION("Scatter produces different directions") {

        auto material = Material::Diffuse({0.5f, 0.5f, 0.5f});

        HitRecord hit{};
        hit.m_point  = Vec3(0.f, 0.f, 0.f);
        hit.m_normal = Vec3(0.f, 1.f, 0.f);

        Ray incoming(
            Vec3(0.f, 10.f, 0.f),
            Vec3(0.f, -1.f, 0.f)
        );

        auto scattered = material.Scatter(
            incoming,
            hit,
            16
        );

        bool found_different = false;

        for (size_t i = 1; i < scattered.size(); i++) {

            const auto& a = scattered[i - 1].m_direction;
            const auto& b = scattered[i].m_direction;

            if ((a - b).length() > 0.0001f) {
                found_different = true;
                break;
            }
        }

        REQUIRE(found_different);
    }
}