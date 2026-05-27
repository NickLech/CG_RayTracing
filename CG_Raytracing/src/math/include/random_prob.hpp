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

	/// <summary>
	/// Return random vector in square centered at
	/// _p (ignores z coordinate), depending on
	/// top left pixel and x,y deltas
	/// </summary>
	/// <param name="_p">The point</param>
	/// <returns>The random vec</returns>
	Vec3 GetRandomVecAroundPoint(Vec3 _p, Vec3 _top_left, float _hoz_offset, float _vert_offset);
}