#pragma once
#include <cmath>

namespace cg_raytracing {
namespace math {
class Vec3 {
  public:
    float x, y, z;
    Vec3(float _x = 0.0, float _y = 0.0, float _z = 0.0) : x(_x), y(_y), z(_z) {};

    Vec3 operator-() const;

    Vec3 operator*(const Vec3& _other) const;

    Vec3 operator+(const Vec3 &_other) const;
    Vec3& operator+=(const Vec3 &_other);
    Vec3 operator-(const Vec3 &_other) const;
    Vec3 operator*(const float _scalar) const;
    Vec3& operator*=(const float _scalar);
    Vec3 operator/(const float _scalar) const;
    Vec3& operator/=(const float _scalar);
    Vec3& operator=(const Vec3 &_other);

    double DotProduct(const Vec3 &_other) const;
    double  dot(const Vec3 &_other) const;
    Vec3   CrossProduct(const Vec3 &_other) const;

    double length_squared() const;
    double length() const;
    Vec3  normalized() const;


    void Rotate(const Vec3 &_rotation_angels);

};
} // namespace math
} // namespace cg_raytracing
