#include <kd_tree.hpp>
#include <cube.hpp>
#include <material.hpp>
#include <vec3.hpp>

#include <vector>
#include <memory>
#include <random>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_assertion_result.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("KD Tree tests") {
	SECTION("Single object intersection") {
		using Cube = cg_raytracing::geometry::Cube;
		using Hittable = cg_raytracing::geometry::Hittable;
		using Vec3 = cg_raytracing::math::Vec3;
		using Ray = cg_raytracing::math::Ray;

		std::vector<std::shared_ptr<Hittable>> objects{};
		auto material = std::make_shared<cg_raytracing::geometry::StandardMaterial>(
			cg_raytracing::geometry::StandardMaterial::Diffuse({0.5f, 0.5f, 0.5f})
		);
		objects.push_back(std::make_shared<Cube>(Vec3(.0f, .0f, .0f), 5.f, material));

		auto kd_tree = cg_raytracing::geometry::KDTree::CreateFromHittables(objects, 50.f);

		// Test ray INSIDE the object
		auto intersected = kd_tree.RayIntersectsObjects(Ray(Vec3(0.f, .0f, .0f), Vec3(1.f, .0f, .0f)));
		REQUIRE(intersected.size() == 1);
		// Test negative x towards positive x
		intersected = kd_tree.RayIntersectsObjects(Ray(Vec3(-10.f, .0f, .0f), Vec3(1.f, .0f, .0f)));
		REQUIRE(intersected.size() == 1);
		// Test positive x towards positive x
		intersected = kd_tree.RayIntersectsObjects(Ray(Vec3(20.f, .0f, .0f), Vec3(1.f, .0f, .0f)));
		REQUIRE(intersected.size() == 0);
		// Test neg y towards positive y
		intersected = kd_tree.RayIntersectsObjects(Ray(Vec3(0.f, -20.0f, .0f), Vec3(0.f, 1.0f, .0f)));
		REQUIRE(intersected.size() == 1);
		// Test pos y towards positive y
		intersected = kd_tree.RayIntersectsObjects(Ray(Vec3(0.f, 20.0f, .0f), Vec3(0.f, 1.0f, .0f)));
		REQUIRE(intersected.size() == 0);
		// Test pos y towards neg y
		intersected = kd_tree.RayIntersectsObjects(Ray(Vec3(0.f, 20.0f, .0f), Vec3(0.f, -1.0f, .0f)));
		REQUIRE(intersected.size() == 1);
		// Test neg xyz towards positive xyz
		intersected = kd_tree.RayIntersectsObjects(Ray(Vec3(-20.f, -20.0f, -20.0f), Vec3(1.f, 1.0f, 1.0f)));
		REQUIRE(intersected.size() == 1);
		// Test pos xyz towards positive xyz
		intersected = kd_tree.RayIntersectsObjects(Ray(Vec3(20.f, 20.0f, 20.0f), Vec3(1.f, 1.0f, 1.0f)));
		REQUIRE(intersected.size() == 0);
		// Test pos xyz towards neg xyz
		intersected = kd_tree.RayIntersectsObjects(Ray(Vec3(20.f, 20.0f, 20.0f), Vec3(-1.f, -1.0f, -1.0f)));
		REQUIRE(intersected.size() == 1);
	}

	SECTION("Random objects") {
		std::random_device rd;
		std::mt19937 gen(rd()); 

		const float MIN = 10.f;
		const float MAX = 90.f;
		std::uniform_real_distribution<float> generator_x{MIN, MAX};
		std::uniform_real_distribution<float> generator_y{MIN, MAX};
		std::uniform_real_distribution<float> generator_z{MIN, MAX};

		const size_t NUM_CUBES = 1000;

		using Cube = cg_raytracing::geometry::Cube;
		using Hittable = cg_raytracing::geometry::Hittable;
		using Vec3 = cg_raytracing::math::Vec3;
		using Ray = cg_raytracing::math::Ray;

		auto material = std::make_shared<cg_raytracing::geometry::StandardMaterial>(
			cg_raytracing::geometry::StandardMaterial::Diffuse({0.5f, 0.5f, 0.5f})
		);

		const float HALF_SIZE = 2.5f;

		std::vector<std::shared_ptr<Hittable>> objects{};
		for (size_t i = 0; i < NUM_CUBES; i++) {
			auto pos_x = generator_x(gen);
			auto pos_y = generator_y(gen);
			auto pos_z = generator_z(gen);
			objects.push_back(std::make_shared<Cube>(Vec3(pos_x, pos_y, pos_z), HALF_SIZE, material));
		}

		auto kd_tree = cg_raytracing::geometry::KDTree::CreateFromHittables(objects, 200.f);

		REQUIRE(kd_tree.GetObjectCount() == NUM_CUBES);
	}

	SECTION("Single object KNN") {
		using Cube = cg_raytracing::geometry::Cube;
		using Hittable = cg_raytracing::geometry::Hittable;
		using Vec3 = cg_raytracing::math::Vec3;
		using Ray = cg_raytracing::math::Ray;

		std::vector<std::shared_ptr<Hittable>> objects{};
		auto material = std::make_shared<cg_raytracing::geometry::StandardMaterial>(
			cg_raytracing::geometry::StandardMaterial::Diffuse({0.5f, 0.5f, 0.5f})
		);
		objects.push_back(std::make_shared<Cube>(Vec3(.0f, .0f, .0f), 5.f, material));

		auto kd_tree = cg_raytracing::geometry::KDTree::CreateFromHittables(objects, 50.f);

		auto knn = kd_tree.NearestNeighbours(Vec3(.0f, .0f, .0f), 2.5f);
		REQUIRE(knn.size() == 1);
		knn = kd_tree.NearestNeighbours(Vec3(6.0f, 6.0f, 6.0f), 3.f);
		REQUIRE(knn.size() == 1);
		knn = kd_tree.NearestNeighbours(Vec3(8.0f, 8.0f, 8.0f), 3.f);
		REQUIRE(knn.size() == 0);
	}

	SECTION("Four object KNN") {
		using Cube = cg_raytracing::geometry::Cube;
		using Hittable = cg_raytracing::geometry::Hittable;
		using Vec3 = cg_raytracing::math::Vec3;
		using Ray = cg_raytracing::math::Ray;

		std::vector<std::shared_ptr<Hittable>> objects{};
		auto material = std::make_shared<cg_raytracing::geometry::StandardMaterial>(
			cg_raytracing::geometry::StandardMaterial::Diffuse({0.5f, 0.5f, 0.5f})
		);
		objects.push_back(std::make_shared<Cube>(Vec3(.0f, .0f, .0f), 2.5f, material));
		objects.push_back(std::make_shared<Cube>(Vec3(10.0f, .0f, .0f), 2.5f, material));

		objects.push_back(std::make_shared<Cube>(Vec3(.0f, .0f, 10.0f), 2.5f, material));
		objects.push_back(std::make_shared<Cube>(Vec3(10.0f, .0f, 10.0f), 2.5f, material));

		auto kd_tree = cg_raytracing::geometry::KDTree::CreateFromHittables(objects, 50.f);

		auto knn = kd_tree.NearestNeighbours(Vec3(6.0f, 6.0f, 6.0f), 5.f);
		REQUIRE(knn.size() == 1);
		REQUIRE(knn[0].first->obj_index.value() == 3);
		knn = kd_tree.NearestNeighbours(Vec3(6.0f, 6.0f, 6.0f), 7.f);
		REQUIRE(knn.size() == 4);
		REQUIRE(knn[0].first->obj_index.value() == 3);
		REQUIRE(knn[1].first->obj_index.value() == 1);
		REQUIRE(knn[2].first->obj_index.value() == 2);
		REQUIRE(knn[3].first->obj_index.value() == 0);

		knn = kd_tree.NearestNeighbours(Vec3(5.0f, 0.0f, 0.0f), 3.f);
		REQUIRE(knn.size() == 2);
		REQUIRE(knn[0].first->obj_index.value() == 0);
		REQUIRE(knn[1].first->obj_index.value() == 1);

		knn = kd_tree.NearestNeighbours(Vec3(0.0f, 0.0f, 5.0f), 3.f);
		REQUIRE(knn.size() == 2);
		REQUIRE(knn[0].first->obj_index.value() == 0);
		REQUIRE(knn[1].first->obj_index.value() == 2);

		knn = kd_tree.NearestNeighbours(Vec3(10.0f, 0.0f, 5.0f), 3.f);
		REQUIRE(knn.size() == 2);
		REQUIRE(knn[0].first->obj_index.value() == 1);
		REQUIRE(knn[1].first->obj_index.value() == 3);

		knn = kd_tree.NearestNeighbours(Vec3(5.0f, 0.0f, 10.0f), 3.f);
		REQUIRE(knn.size() == 2);
		REQUIRE(knn[0].first->obj_index.value() == 2);
		REQUIRE(knn[1].first->obj_index.value() == 3);

		knn = kd_tree.NearestNeighbours(Vec3(0.0f, 0.0f, 10.0f), 2.5f);
		REQUIRE(knn.size() == 1);
		REQUIRE(knn[0].first->obj_index.value() == 2);
	}

	SECTION("Four object ray intersect") {
		using Cube = cg_raytracing::geometry::Cube;
		using Hittable = cg_raytracing::geometry::Hittable;
		using Vec3 = cg_raytracing::math::Vec3;
		using Ray = cg_raytracing::math::Ray;

		std::vector<std::shared_ptr<Hittable>> objects{};
		auto material = std::make_shared<cg_raytracing::geometry::StandardMaterial>(
			cg_raytracing::geometry::StandardMaterial::Diffuse({0.5f, 0.5f, 0.5f})
		);
		objects.push_back(std::make_shared<Cube>(Vec3(.0f, .0f, .0f), 2.5f, material));
		objects.push_back(std::make_shared<Cube>(Vec3(10.0f, .0f, .0f), 2.5f, material));

		objects.push_back(std::make_shared<Cube>(Vec3(.0f, .0f, 10.0f), 2.5f, material));
		objects.push_back(std::make_shared<Cube>(Vec3(10.0f, .0f, 10.0f), 2.5f, material));

		auto kd_tree = cg_raytracing::geometry::KDTree::CreateFromHittables(objects, 50.f);

		// Middle between 0 and 1 towards nothing
		auto hit = kd_tree.RayIntersectsObjects(Ray(Vec3(5.0f, 0.0f, 0.0f), Vec3(.0f, .0f, 1.f)));
		REQUIRE(hit.size() == 0);
		// Before 0 towards 0 and 1
		hit = kd_tree.RayIntersectsObjects(Ray(Vec3(-5.0f, 0.0f, 0.0f), Vec3(1.0f, .0f, .0f)));
		REQUIRE(hit.size() == 2);
		REQUIRE(hit[0].first->obj_index.value() == 0);
		REQUIRE(hit[1].first->obj_index.value() == 1);
		// Before 2 towards 2 and 3
		hit = kd_tree.RayIntersectsObjects(Ray(Vec3(-5.0f, 0.0f, 10.0f), Vec3(1.0f, .0f, .0f)));
		REQUIRE(hit.size() == 2);
		REQUIRE(hit[0].first->obj_index.value() == 2);
		REQUIRE(hit[1].first->obj_index.value() == 3);
		
		hit = kd_tree.RayIntersectsObjects(Ray(Vec3(0.0f, 0.0f, -5.0f), Vec3(.0f, .0f, 1.0f)));
		REQUIRE(hit.size() == 2);
		REQUIRE(hit[0].first->obj_index.value() == 0);
		REQUIRE(hit[1].first->obj_index.value() == 2);

		hit = kd_tree.RayIntersectsObjects(Ray(Vec3(10.0f, 0.0f, -5.0f), Vec3(.0f, .0f, 1.0f)));
		REQUIRE(hit.size() == 2);
		REQUIRE(hit[0].first->obj_index.value() == 1);
		REQUIRE(hit[1].first->obj_index.value() == 3);

		// Same as above, but opposite direction, reversed object order
		hit = kd_tree.RayIntersectsObjects(Ray(Vec3(10.0f, 0.0f, 15.0f), Vec3(.0f, .0f, -1.0f)));
		REQUIRE(hit.size() == 2);
		REQUIRE(hit[0].first->obj_index.value() == 3);
		REQUIRE(hit[1].first->obj_index.value() == 1);

		hit = kd_tree.RayIntersectsObjects(Ray(Vec3(-5.0f, 0.0f, -5.0f), Vec3(1.0f, .0f, 1.0f)));
		REQUIRE(hit.size() == 2);
		REQUIRE(hit[0].first->obj_index.value() == 0);
		REQUIRE(hit[1].first->obj_index.value() == 3);

		hit = kd_tree.RayIntersectsObjects(Ray(Vec3(15.0f, 0.0f, 15.0f), Vec3(-1.0f, .0f, -1.0f)));
		REQUIRE(hit.size() == 2);
		REQUIRE(hit[0].first->obj_index.value() == 3);
		REQUIRE(hit[1].first->obj_index.value() == 0);

		hit = kd_tree.RayIntersectsObjects(Ray(Vec3(15.0f, 0.0f, -5.0f), Vec3(-1.0f, .0f, 1.0f)));
		REQUIRE(hit.size() == 2);
		REQUIRE(hit[0].first->obj_index.value() == 1);
		REQUIRE(hit[1].first->obj_index.value() == 2);

		hit = kd_tree.RayIntersectsObjects(Ray(Vec3(-5.0f, 0.0f, 15.0f), Vec3(1.0f, .0f, -1.0f)));
		REQUIRE(hit.size() == 2);
		REQUIRE(hit[0].first->obj_index.value() == 2);
		REQUIRE(hit[1].first->obj_index.value() == 1);
	}

	SECTION("Adding objects") {
		using Cube = cg_raytracing::geometry::Cube;
		using Hittable = cg_raytracing::geometry::Hittable;
		using Vec3 = cg_raytracing::math::Vec3;
		using Ray = cg_raytracing::math::Ray;

		std::vector<std::shared_ptr<Hittable>> objects{};
		auto material = std::make_shared<cg_raytracing::geometry::StandardMaterial>(
			cg_raytracing::geometry::StandardMaterial::Diffuse({0.5f, 0.5f, 0.5f})
		);

		objects.push_back(std::make_shared<Cube>(Vec3(.0f, .0f, .0f), 2.5f, material));
		auto kd_tree = cg_raytracing::geometry::KDTree::CreateFromHittables(objects, 50.f);

		objects.push_back(std::make_shared<Cube>(Vec3(10.0f, .0f, .0f), 2.5f, material));
		objects.push_back(std::make_shared<Cube>(Vec3(.0f, .0f, 10.0f), 2.5f, material));
		objects.push_back(std::make_shared<Cube>(Vec3(10.0f, .0f, 10.0f), 2.5f, material));

		kd_tree.AddHittable(objects[1], 1);
		REQUIRE(kd_tree.GetObjectCount() == 2);
		kd_tree.AddHittable(objects[2], 2);
		REQUIRE(kd_tree.GetObjectCount() == 3);
		kd_tree.AddHittable(objects[3], 3);
		REQUIRE(kd_tree.GetObjectCount() == 4);

		// Middle between 0 and 1 towards nothing
		auto hit = kd_tree.RayIntersectsObjects(Ray(Vec3(5.0f, 0.0f, 0.0f), Vec3(.0f, .0f, 1.f)));
		REQUIRE(hit.size() == 0);
		// Before 0 towards 0 and 1
		hit = kd_tree.RayIntersectsObjects(Ray(Vec3(-5.0f, 0.0f, 0.0f), Vec3(1.0f, .0f, .0f)));
		REQUIRE(hit.size() == 2);
		REQUIRE(hit[0].first->obj_index.value() == 0);
		REQUIRE(hit[1].first->obj_index.value() == 1);
		// Before 2 towards 2 and 3
		hit = kd_tree.RayIntersectsObjects(Ray(Vec3(-5.0f, 0.0f, 10.0f), Vec3(1.0f, .0f, .0f)));
		REQUIRE(hit.size() == 2);
		REQUIRE(hit[0].first->obj_index.value() == 2);
		REQUIRE(hit[1].first->obj_index.value() == 3);

		hit = kd_tree.RayIntersectsObjects(Ray(Vec3(0.0f, 0.0f, -5.0f), Vec3(.0f, .0f, 1.0f)));
		REQUIRE(hit.size() == 2);
		REQUIRE(hit[0].first->obj_index.value() == 0);
		REQUIRE(hit[1].first->obj_index.value() == 2);

		hit = kd_tree.RayIntersectsObjects(Ray(Vec3(10.0f, 0.0f, -5.0f), Vec3(.0f, .0f, 1.0f)));
		REQUIRE(hit.size() == 2);
		REQUIRE(hit[0].first->obj_index.value() == 1);
		REQUIRE(hit[1].first->obj_index.value() == 3);

		// Same as above, but opposite direction, reversed object order
		hit = kd_tree.RayIntersectsObjects(Ray(Vec3(10.0f, 0.0f, 15.0f), Vec3(.0f, .0f, -1.0f)));
		REQUIRE(hit.size() == 2);
		REQUIRE(hit[0].first->obj_index.value() == 3);
		REQUIRE(hit[1].first->obj_index.value() == 1);

		hit = kd_tree.RayIntersectsObjects(Ray(Vec3(-5.0f, 0.0f, -5.0f), Vec3(1.0f, .0f, 1.0f)));
		REQUIRE(hit.size() == 2);
		REQUIRE(hit[0].first->obj_index.value() == 0);
		REQUIRE(hit[1].first->obj_index.value() == 3);

		hit = kd_tree.RayIntersectsObjects(Ray(Vec3(15.0f, 0.0f, 15.0f), Vec3(-1.0f, .0f, -1.0f)));
		REQUIRE(hit.size() == 2);
		REQUIRE(hit[0].first->obj_index.value() == 3);
		REQUIRE(hit[1].first->obj_index.value() == 0);

		hit = kd_tree.RayIntersectsObjects(Ray(Vec3(15.0f, 0.0f, -5.0f), Vec3(-1.0f, .0f, 1.0f)));
		REQUIRE(hit.size() == 2);
		REQUIRE(hit[0].first->obj_index.value() == 1);
		REQUIRE(hit[1].first->obj_index.value() == 2);

		hit = kd_tree.RayIntersectsObjects(Ray(Vec3(-5.0f, 0.0f, 15.0f), Vec3(1.0f, .0f, -1.0f)));
		REQUIRE(hit.size() == 2);
		REQUIRE(hit[0].first->obj_index.value() == 2);
		REQUIRE(hit[1].first->obj_index.value() == 1);
	}
}