#include <vec3.hpp>
#include <random>

namespace cg_raytracing::math {
	struct RandomState {
		std::mt19937                           mt_generator{};
	};

	extern thread_local RandomState g_thread_rd_state;

	double GetRandomDouble();
	double GetRandomDouble(double _min, double _max);

	float GetRandomFloat();
	float GetRandomFloat(float _min, float _max);

	Vec3 GetRandomVec3();
	Vec3 GetRandomVec3(float _min, float _max);

	Vec3 GetRandomUnitVec3();

	math::Vec3 RandomInHemisphere(const math::Vec3& _normal);
}