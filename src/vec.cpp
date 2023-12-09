#include <algorithm>
#include <cmath>

#include "vec.hpp"

const Vec3& Vec3::operator+() const {
    return *this;
}
Vec3 Vec3::operator-() const {
    return Vec3(-x, -y, -z);
}

// addition
Vec3 Vec3::operator+(const Vec3 &v2) const {
    return Vec3(x + v2.x, y + v2.y, z + v2.z);
}
Vec3& Vec3::operator+=(const Vec3 &v2) {
    x += v2.x;
    y += v2.y;
    z += v2.z;
    return *this;
}

// subtraction
Vec3 Vec3::operator-(const Vec3 &v2) const {
    return Vec3(x - v2.x, y - v2.y, z - v2.z);
}
Vec3& Vec3::operator-=(const Vec3 &v2) {
    x -= v2.x;
    y -= v2.y;
    z -= v2.z;
    return *this;
}

// multiplication
Vec3 Vec3::operator*(const Vec3 &v2) const {
    return Vec3(x * v2.x, y * v2.y, z * v2.z);
}
Vec3& Vec3::operator*=(const Vec3 &v2) {
    x *= v2.x;
    y *= v2.y;
    z *= v2.z;
    return *this;
}

// division
Vec3 Vec3::operator/(const Vec3 &v2) const {
    return Vec3(x / v2.x, y / v2.y, z / v2.z);
}
Vec3& Vec3::operator/=(const Vec3 &v2) {
    x /= v2.x;
    y /= v2.y;
    z /= v2.z;
    return *this;
}

// scalar multiplication
Vec3 Vec3::operator*(float t) const {
    return Vec3(x * t, y * t, z * t);
}
Vec3& Vec3::operator*=(float t) {
    x *= t;
    y *= t;
    z *= t;
    return *this;
}

// scalar division
Vec3 Vec3::operator/(float t) const {
    return Vec3(x / t, y / t, z / t);
}
Vec3& Vec3::operator/=(float t) {
    x /= t;
    y /= t;
    z /= t;
    return *this;
}

// linalg operations
float Vec3::norm() const {
    return std::sqrt(x * x + y * y + z * z);
}
float Vec3::norm_squared() const {
    return x * x + y * y + z * z;
}

Vec3 Vec3::normalize() const {
    float n = norm();
    return Vec3(x / n, y / n, z / n);
}

float Vec3::dot(const Vec3 &v2) const {
    return x * v2.x + y * v2.y + z * v2.z;
}

Vec3 Vec3::cross(const Vec3 &v2) const {
    return Vec3(
        y * v2.z - z * v2.y,
        z * v2.x - x * v2.z,
        x * v2.y - y * v2.x
    );
}

std::array<float, 4> Vec3::to_homog() const {
    return {x, y, z, 0};
}

Vec3 Vec3::from_homog(const std::array<float, 4> &v) {
    return Vec3(v[0], v[1], v[2]);
}

// ray operations

Vec3 Vec3::reflect(const Vec3 &normal) const {
    return *this - 2.0f * this->dot(normal) * normal;
}

// NOTE: assumes that this and normal are both unit vectors!
Vec3 Vec3::refract(const Vec3 &normal, float ior_ratio) const {
    float cos_theta = std::min(-normal.dot(*this), 1.0f);
    float sin_theta = std::sqrt(1.0f - cos_theta * cos_theta);
    if (ior_ratio * sin_theta > 1.0f) {
        return reflect(normal);
    }
    Vec3 r_out_perp = ior_ratio * (cos_theta * normal + *this);
    Vec3 r_out_parallel = -std::sqrt(std::abs(1.0f - r_out_perp.norm_squared())) * normal;
    
    return r_out_perp + r_out_parallel;
}

Vec3 operator*(float t, const Vec3 &v) {
    return v * t;
}


Pt3 Pt3::operator+(const Vec3 &v2) const {
    return Pt3(x + v2.x, y + v2.y, z + v2.z);
}
Pt3& Pt3::operator+=(const Vec3 &v2) {
    x += v2.x;
    y += v2.y;
    z += v2.z;
    return *this;
}

Pt3 Pt3::operator-(const Vec3 &v2) const {
    return Pt3(x - v2.x, y - v2.y, z - v2.z);
}
Pt3& Pt3::operator-=(const Vec3 &v2) {
    x -= v2.x;
    y -= v2.y;
    z -= v2.z;
    return *this;
}

std::array<float, 4> Pt3::to_homog() const {
    return {x, y, z, 1};
}

Pt3 Pt3::from_homog(const std::array<float, 4> &v) {
    return Pt3(v[0] / v[3], v[1] / v[3], v[2] / v[3]);
}


Vec2 Vec2::operator-() const {
    return Vec2(-x, -y);
}

Vec2 Vec2::operator+(const Vec2 &v2) const {
    return Vec2(x + v2.x, y + v2.y);
}
Vec2& Vec2::operator+=(const Vec2 &v2) {
    x += v2.x;
    y += v2.y;
    return *this;
}

Vec2 Vec2::operator-(const Vec2 &v2) const {
    return Vec2(x - v2.x, y - v2.y);
}
Vec2& Vec2::operator-=(const Vec2 &v2) {
    x -= v2.x;
    y -= v2.y;
    return *this;
}

Vec2 Vec2::operator*(float t) const {
    return Vec2(x * t, y * t);
}
Vec2& Vec2::operator*=(float t) {
    x *= t;
    y *= t;
    return *this;
}

Vec2 Vec2::operator/(float t) const {
    return Vec2(x / t, y / t);
}
Vec2& Vec2::operator/=(float t) {
    x /= t;
    y /= t;
    return *this;
}

Vec2 operator*(float t, const Vec2& v) {
    return v * t;
}
