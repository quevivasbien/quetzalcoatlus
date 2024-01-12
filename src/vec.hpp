#pragma once

#include <array>
#include <string>

class Vec3 {
public:
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    float r() const {
        return x;
    }
    float g() const {
        return y;
    }
    float b() const {
        return z;
    }

    std::string str() const;

    bool is_zero() const;

    const Vec3& operator+() const;
    Vec3 operator-() const;

    bool operator==(const Vec3 &v2) const;

    // addition
    Vec3 operator+(const Vec3 &v2) const;
    Vec3& operator+=(const Vec3 &v2);

    // subtraction
    Vec3 operator-(const Vec3 &v2) const;
    Vec3& operator-=(const Vec3 &v2);

    // multiplication
    Vec3 operator*(const Vec3 &v2) const;
    Vec3& operator*=(const Vec3 &v2);

    // division
    Vec3 operator/(const Vec3 &v2) const;
    Vec3& operator/=(const Vec3 &v2);

    // scalar multiplication
    Vec3 operator*(float t) const;
    Vec3& operator*=(float t);

    // scalar division
    Vec3 operator/(float t) const;
    Vec3& operator/=(float t);

    // linalg operations
    float norm() const;
    float norm_squared() const;

    // return a new normalized vector
    Vec3 normalized() const;
    // normalize in place
    Vec3& normalize();

    float dot(const Vec3 &v2) const;

    Vec3 cross(const Vec3 &v2) const;

    std::array<float, 4> to_homog() const;
    static Vec3 from_homog(const std::array<float, 4> &v);

    template <typename F>
    Vec3 map(F f) const {
        return Vec3(f(x), f(y), f(z));
    }

    float x;
    float y;
    float z;
};

Vec3 operator*(float t, const Vec3 &v);


class Pt3 : public Vec3 {
public:
    Pt3() : Vec3() {}
    Pt3(float x, float y, float z) : Vec3(x, y, z) {}
    explicit Pt3(Vec3&& v) : Vec3(std::move(v)) {}

    Pt3 operator+(const Vec3 &v2) const;
    Pt3& operator+=(const Vec3 &v2);

    Pt3 operator-(const Vec3 &v2) const;
    Pt3& operator-=(const Vec3 &v2);

    std::array<float, 4> to_homog() const;
    static Pt3 from_homog(const std::array<float, 4> &v);
};


class Vec2 {
public:
    float x;
    float y;

    Vec2() : x(0.0f), y(0.0f) {}

    Vec2(float x, float y) : x(x), y(y) {}

    Vec2 operator-() const;

    Vec2 operator+(const Vec2 &v2) const;
    Vec2& operator+=(const Vec2 &v2);

    Vec2 operator-(const Vec2 &v2) const;
    Vec2& operator-=(const Vec2 &v2);

    Vec2 operator*(float t) const;
    Vec2& operator*=(float t);

    Vec2 operator/(float t) const;
    Vec2& operator/=(float t);

    float norm_squared() const;
    float norm() const;
};

Vec2 operator*(float t, const Vec2 &v);
