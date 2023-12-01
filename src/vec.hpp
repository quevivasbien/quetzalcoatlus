#pragma once

#include <cmath>
#include <iostream>

class Vec3 {
public:
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    inline float r() const {
        return x;
    }
    inline float g() const {
        return y;
    }
    inline float b() const {
        return z;
    }

    inline const Vec3& operator+() const {
        return *this;
    }
    inline Vec3 operator-() const {
        return Vec3(-x, -y, -z);
    }

    // addition
    inline Vec3 operator+(const Vec3 &v2) const {
        return Vec3(x + v2.x, y + v2.y, z + v2.z);
    }
    inline Vec3& operator+=(const Vec3 &v2) {
        x += v2.x;
        y += v2.y;
        z += v2.z;
        return *this;
    }

    // subtraction
    inline Vec3 operator-(const Vec3 &v2) const {
        return Vec3(x - v2.x, y - v2.y, z - v2.z);
    }
    inline Vec3& operator-=(const Vec3 &v2) {
        x -= v2.x;
        y -= v2.y;
        z -= v2.z;
        return *this;
    }

    // multiplication
    inline Vec3 operator*(const Vec3 &v2) const {
        return Vec3(x * v2.x, y * v2.y, z * v2.z);
    }
    inline Vec3& operator*=(const Vec3 &v2) {
        x *= v2.x;
        y *= v2.y;
        z *= v2.z;
        return *this;
    }

    // division
    inline Vec3 operator/(const Vec3 &v2) const {
        return Vec3(x / v2.x, y / v2.y, z / v2.z);
    }
    inline Vec3& operator/=(const Vec3 &v2) {
        x /= v2.x;
        y /= v2.y;
        z /= v2.z;
        return *this;
    }

    // scalar multiplication
    inline Vec3 operator*(float t) const {
        return Vec3(x * t, y * t, z * t);
    }
    inline Vec3& operator*=(float t) {
        x *= t;
        y *= t;
        z *= t;
        return *this;
    }

    // scalar division
    inline Vec3 operator/(float t) const {
        return Vec3(x / t, y / t, z / t);
    }
    inline Vec3& operator/=(float t) {
        x /= t;
        y /= t;
        z /= t;
        return *this;
    }

    // linalg operations
    inline float norm() const {
        return sqrt(x * x + y * y + z * z);
    }
    inline float norm_squared() const {
        return x * x + y * y + z * z;
    }

    inline Vec3 normalize() const {
        float n = norm();
        return Vec3(x / n, y / n, z / n);
    }

    inline float dot(const Vec3 &v2) const {
        return x * v2.x + y * v2.y + z * v2.z;
    }

    inline Vec3 cross(const Vec3 &v2) const {
        return Vec3(
            y * v2.z - z * v2.y,
            z * v2.x - x * v2.z,
            x * v2.y - y * v2.x
        );
    }

    template <typename F>
    inline Vec3 map(F f) const {
        return Vec3(f(x), f(y), f(z));
    }

    float x;
    float y;
    float z;
};

inline Vec3 operator*(float t, const Vec3 &v) {
    return v * t;
}


class Pt3 : public Vec3 {
public:
    Pt3() : Vec3() {}
    Pt3(float x, float y, float z) : Vec3(x, y, z) {}

    inline Pt3 operator+(const Vec3 &v2) const {
        return Pt3(x + v2.x, y + v2.y, z + v2.z);
    }
    inline Pt3& operator+=(const Vec3 &v2) {
        x += v2.x;
        y += v2.y;
        z += v2.z;
        return *this;
    }

    inline Pt3 operator-(const Vec3 &v2) const {
        return Pt3(x - v2.x, y - v2.y, z - v2.z);
    }
    inline Pt3& operator-=(const Vec3 &v2) {
        x -= v2.x;
        y -= v2.y;
        z -= v2.z;
        return *this;
    }
};


class Vec2 {
public:
    float x;
    float y;

    Vec2() : x(0.0f), y(0.0f) {}

    Vec2(float x, float y) : x(x), y(y) {}

    inline Vec2 operator-() const {
        return Vec2(-x, -y);
    }

    inline Vec2 operator+(const Vec2 &v2) const {
        return Vec2(x + v2.x, y + v2.y);
    }
    inline Vec2& operator+=(const Vec2 &v2) {
        x += v2.x;
        y += v2.y;
        return *this;
    }

    inline Vec2 operator-(const Vec2 &v2) const {
        return Vec2(x - v2.x, y - v2.y);
    }
    inline Vec2& operator-=(const Vec2 &v2) {
        x -= v2.x;
        y -= v2.y;
        return *this;
    }

    inline Vec2 operator*(float t) const {
        return Vec2(x * t, y * t);
    }
    inline Vec2& operator*=(float t) {
        x *= t;
        y *= t;
        return *this;
    }

    inline Vec2 operator/(float t) const {
        return Vec2(x / t, y / t);
    }
    inline Vec2& operator/=(float t) {
        x /= t;
        y /= t;
        return *this;
    }
};
