#include <random_prob.hpp>

namespace cg_raytracing::math {
	thread_local RandomState g_thread_rd_state{};

	double GetRandomDouble() {
		static std::uniform_real_distribution<double> dist{ .0, 1.0 };
		return dist(g_thread_rd_state.mt_generator);
	}

	double GetRandomDouble(double _min, double _max) {
		auto norm = GetRandomDouble();
		return _min + (_max - _min) * norm;
	}

	float GetRandomFloat() {
		static std::uniform_real_distribution<float> dist{ .0f, 1.0f };
		return dist(g_thread_rd_state.mt_generator);
	}

	float GetRandomFloat(float _min, float _max) {
		auto norm = GetRandomFloat();
		return _min + (_max - _min) * norm;
	}

	Vec3 GetRandomVec3() {
		return Vec3(GetRandomFloat(), GetRandomFloat(), GetRandomFloat());
	}

	Vec3 GetRandomVec3(float _min, float _max) {
		return Vec3(GetRandomFloat(_min, _max), GetRandomFloat(_min, _max), GetRandomFloat(_min, _max));
	}
	
	Vec3 GetRandomUnitVec3() {
		auto vec = GetRandomVec3(-1.f, 1.f);
		if (0.f != vec.length()) {
			vec = vec.normalized();
		}
		return vec;
	}

	/// Generates a uniformly distributed random unit vector in the hemisphere
	/// oriented around the given surface normal.
	math::Vec3 RandomInHemisphere(const math::Vec3& _normal) {
		// Seeded once per program run — mt19937 is a pseudo-random number generator
		// static std::mt19937 rng(std::random_device{}());
		// static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
		// Do NOT use local static rng, it is not thread safe

		math::Vec3 random_dir;
		// Rejection sampling: discard points outside the unit sphere
		do {
			random_dir = math::Vec3(GetRandomFloat(), GetRandomFloat(), GetRandomFloat());
		} while (random_dir.length_squared() > 1.0f || random_dir.length_squared() < 1e-16);

		// Flip to correct hemisphere if pointing away from the surface (under the surface)
		if (random_dir.dot(_normal) < 0.0f)
			random_dir = random_dir * -1.0f;

		return random_dir.normalized();
	}

	Vec3 GetRandomVecAroundPoint(Vec3 _p, Vec3 _top_left, float _hoz_offset, float _vert_offset) {
		auto sample_square = [_p]() {
			return Vec3(GetRandomFloat() - .5f, GetRandomFloat() - .5f, .0f);
		};

		const auto offset = sample_square();
		
		return _top_left + (offset + _p) * Vec3(_hoz_offset, _vert_offset, .0f);
	}
}