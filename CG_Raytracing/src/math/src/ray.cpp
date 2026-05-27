#include "ray.hpp"
#include "vec3.hpp"

#include <cmath>

using namespace cg_raytracing::math;

Ray::Ray(Vec3 _origin, Vec3 _direction) {
    this->m_origin = _origin;
    this->m_direction = _direction;
}

void Ray::SetDirection(Vec3 const &_other) {
    this->m_direction = _other;
}

void Ray::SetOrigin(Vec3 const &_other) {
    this->m_origin = _other;
}

void Ray::Simulate() {
    this->m_color = Vec3(0.0, 0.0, 0.0);
}

void Ray::Rotate(const Vec3 &_rotation_angles) {
    this->m_direction.Rotate(_rotation_angles);
}


void Ray::Translate(const math::Vec3 &_translation_vector) {
    this->m_origin += _translation_vector;
}

/*
constexpr float origin()      { return 1.0f / 32.0f; }
constexpr float float_scale() { return 1.0f / 65536.0f; }
constexpr float int_scale()   { return 256.0f; }

// Normal points outward for rays exiting the surface, else is flipped.
float3 offset_ray(const float3 p, const float3 n)
{
  int3 of_i(int_scale() * n.x, int_scale() * n.y, int_scale() * n.z);

  float3 p_i(
      int_as_float(float_as_int(p.x)+((p.x < 0) ? -of_i.x : of_i.x)),
      int_as_float(float_as_int(p.y)+((p.y < 0) ? -of_i.y : of_i.y)),
      int_as_float(float_as_int(p.z)+((p.z < 0) ? -of_i.z : of_i.z)));

  return float3(fabsf(p.x) < origin() ? p.x+ float_scale()*n.x : p_i.x,
                fabsf(p.y) < origin() ? p.y+ float_scale()*n.y : p_i.y,
                fabsf(p.z) < origin() ? p.z+ float_scale()*n.z : p_i.z);
}
*/
void Ray::OffsetAlongNormal(Vec3 const& _n) {
    auto origin = []() { return 1.f / 32.f; };
    auto fscale = []() { return 1.f / 65536.f; };
    auto iscale = []() { return 256.f; };

    auto of_x = (int32_t)(iscale() * _n.x);
    auto of_y = (int32_t)(iscale() * _n.y);
    auto of_z = (int32_t)(iscale() * _n.z);

    auto p_x = (float)((int32_t)m_origin.x + ((m_origin.x < 0) ? -of_x : of_x));
    auto p_y = (float)((int32_t)m_origin.y + ((m_origin.y < 0) ? -of_y : of_y));
    auto p_z = (float)((int32_t)m_origin.z + ((m_origin.z < 0) ? -of_z : of_z));

    m_origin = Vec3(
        std::fabsf(m_origin.x) < origin() ? m_origin.x + fscale() * _n.x : p_x,
        std::fabsf(m_origin.y) < origin() ? m_origin.y + fscale() * _n.y : p_y,
        std::fabsf(m_origin.z) < origin() ? m_origin.z + fscale() * _n.z : p_z
    );
}
