#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <optional>

#include "ray.hpp"
#include "vec.hpp"


struct Mat3 {
    // data is stored in row-major order
    std::array<float, 9> data;

    Mat3() : data({0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}) {};
    explicit Mat3(std::array<float, 9>&& data) : data(std::move(data)) {}

    float operator[] (size_t i) const { return data[i]; }
    float& operator[] (size_t i) { return data[i]; }

    Mat3 operator* (const Mat3& other) const;

    std::array<float, 3> operator* (const std::array<float, 3>& v) const;

    Vec3 operator* (const Vec3& v) const;

    // gets the inverse of the matrix, or returns nullopt if the matrix is not invertible (or nearly so)
    std::optional<Mat3> invert() const;

    static Mat3 identity();

    static Mat3 diagonal(const std::array<float, 3>& v);
};

struct Mat4 {
    // data is stored in row-major order
    std::array<float, 16> data;

    Mat4() : data({
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f}) {};

    explicit Mat4(std::array<float, 16>&& data) : data(std::move(data)) {}

    float operator[] (size_t i) const { return data[i]; }
    float& operator[] (size_t i) { return data[i]; }

    Mat4 operator* (const Mat4& other) const;

    std::array<float, 4> operator* (const std::array<float, 4>& v) const;

    // multiplies as if 4th coordinate is 0
    Vec3 operator* (const Vec3& v) const;

    // multiplies as if 4th coordinate is 1
    Pt3 operator* (const Pt3& v) const;

    static Mat4 identity();

    static Mat4 diagonal(const std::array<float, 4>& v);

    static Mat4 rotate_x(float angle);
    static Mat4 rotate_y(float angle);
    static Mat4 rotate_z(float angle);
    // rotation about a unit vector axis
    static Mat4 rotation(const Vec3& axis, float angle);
};

class Transform {
public:
    Transform(Mat4&& matrix, Mat4&& inverse_matrix) : m_mat(matrix), m_inv_mat(inverse_matrix) {}

    virtual ~Transform() {}

    Vec3 apply(const Vec3& v) const;
    Pt3 apply(const Pt3& p) const;
    Ray apply(const Ray& r) const;

    Vec3 apply_inverse(const Vec3& v) const;
    Pt3 apply_inverse(const Pt3& v) const;
    Ray apply_inverse(const Ray& r) const;

    Transform apply(const Transform& t) const;

    template <typename T>
    T operator*(const T& t) const {
        return apply(t);
    }

    static Transform identity();

    static Transform translation(const Vec3& v);
    static Transform translation(float x, float y, float z);

    static Transform scale(const Vec3& v);
    static Transform scale(float x, float y, float z);
    static Transform scale(float c);

    static Transform rotate_x(float angle);
    static Transform rotate_y(float angle);
    static Transform rotate_z(float angle);
    static Transform rotation(const Vec3& axis, float angle);

    Mat4 m_mat;
    Mat4 m_inv_mat;
};
