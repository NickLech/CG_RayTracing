#include "vec3.hpp"
#include <cmath>
#include <stdexcept>

using namespace cg_raytracing::math;

Vec3 cg_raytracing::math::Vec3::operator-() const {
    return Vec3(-x, -y, -z);
}

Vec3 Vec3::operator*(const Vec3 &_other) const {
    return Vec3(
        this->x * _other.x,
        this->y * _other.y,
        this->z * _other.z
    );
}

Vec3 Vec3::operator+(const Vec3 &_other) const {
    return Vec3(_other.x + this->x, _other.y + this->y, _other.z + this->z);
}

Vec3 &Vec3::operator+=(const Vec3 &_other) {
    this->x += _other.x;
    this->y += _other.y;
    this->z += _other.z;

    return *this;
}

Vec3 Vec3::operator*(const float _scalar) const {
    return Vec3(this->x * _scalar, this->y * _scalar, this->z * _scalar);
}

Vec3 &Vec3::operator*=(const float _scalar) {
    this->x *= _scalar;
    this->y *= _scalar;
    this->z *= _scalar;

    return *this;
}

Vec3 Vec3::operator/(const float _scalar) const {
    if (_scalar == 0) {
        throw std::runtime_error("Division by zero");
    }
    return Vec3(this->x / _scalar, this->y / _scalar, this->z / _scalar);
};

Vec3 &Vec3::operator/=(const float _scalar) {
    if (_scalar == 0) {
        throw std::runtime_error("Division by zero");
    }
    return *this *= 1 / _scalar;
};

Vec3 &Vec3::operator=(const Vec3 &_other) {
    this->x = _other.x;
    this->y = _other.y;
    this->z = _other.z;

    return *this;
}

double Vec3::DotProduct(const Vec3 &_other) const {
    return (double)this->x * _other.x + this->y * _other.y + this->z * _other.z;
}
Vec3 Vec3::CrossProduct(const Vec3 &_other) const {
    return Vec3(this->y * _other.z - this->z * _other.y,
                this->z * _other.x - this->x * _other.z,
                this->x * _other.y - this->y * _other.x);
}

Vec3 Vec3::operator-(const Vec3 &_other) const {
    return Vec3(this->x - _other.x, this->y - _other.y, this->z - _other.z);
}

double Vec3::dot(const Vec3 &_other) const {
    return (double)this->x * _other.x + this->y * _other.y + this->z * _other.z;
}

double Vec3::length_squared() const {
    return (double)this->x * this->x + this->y * this->y + this->z * this->z;
}

double Vec3::length() const {
    return std::sqrt(length_squared());
}

Vec3 Vec3::normalized() const {
    float len = length();
    if (len == 0.0f)
        return Vec3(0, 0, 0);
    return *this / len;
}

void Vec3::Rotate(const Vec3 &_rotation_angles) {
    // rotation around x
    float old_y = this->y;
    float old_z = this->z;
    this->y = old_y * std::cos(_rotation_angles.x) +
              old_z * (-1) * std::sin(_rotation_angles.x);
    this->z = old_y * std::sin(_rotation_angles.x) +
              old_z * std::cos(_rotation_angles.x);

    // rotation around y
    float old_x = this->x;
    old_z = this->z;
    this->x = old_x * std::cos(_rotation_angles.y) +
              old_z * std::sin(_rotation_angles.y);
    this->z = old_x * (-1) * std::sin(_rotation_angles.y) +
              old_z * std::cos(_rotation_angles.y);

    // rotation around z
    old_x = this->x;
    old_y = this->y;
    this->x = old_x * std::cos(_rotation_angles.z) +
              old_y * (-1) * std::sin(_rotation_angles.z);
    this->y = old_x * std::sin(_rotation_angles.z) +
              old_y * std::cos(_rotation_angles.z);
}
