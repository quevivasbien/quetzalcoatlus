#pragma once


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

    const Vec3& operator+() const;
    Vec3 operator-() const;

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

    Vec3 normalize() const;

    float dot(const Vec3 &v2) const;

    Vec3 cross(const Vec3 &v2) const;

    // ray operations

    Vec3 reflect(const Vec3 &normal) const;

    // NOTE: assumes that this and normal are both unit vectors!
    Vec3 refract(const Vec3 &normal, float ior_ratio) const;

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

    Pt3 operator+(const Vec3 &v2) const;
    Pt3& operator+=(const Vec3 &v2);

    Pt3 operator-(const Vec3 &v2) const;
    Pt3& operator-=(const Vec3 &v2);
};


class Vec2 {
public:
    float x;
    float y;

    Vec2() : x(0.0f), y(0.0f) {}

    Vec2(float x, float y) : x(x), y(y) {}

    Vec2 operator-() const {
        return Vec2(-x, -y);
    }

    Vec2 operator+(const Vec2 &v2) const {
        return Vec2(x + v2.x, y + v2.y);
    }
    Vec2& operator+=(const Vec2 &v2) {
        x += v2.x;
        y += v2.y;
        return *this;
    }

    Vec2 operator-(const Vec2 &v2) const {
        return Vec2(x - v2.x, y - v2.y);
    }
    Vec2& operator-=(const Vec2 &v2) {
        x -= v2.x;
        y -= v2.y;
        return *this;
    }

    Vec2 operator*(float t) const {
        return Vec2(x * t, y * t);
    }
    Vec2& operator*=(float t) {
        x *= t;
        y *= t;
        return *this;
    }

    Vec2 operator/(float t) const {
        return Vec2(x / t, y / t);
    }
    Vec2& operator/=(float t) {
        x /= t;
        y /= t;
        return *this;
    }
};
