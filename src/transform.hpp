#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <optional>

#include "vec.hpp"

struct Mat3 {
    // data is stored in row-major order
    std::array<float, 9> data;

    Mat3() {};
    explicit Mat3(std::array<float, 9>&& data) : data(std::move(data)) {}

    float operator[] (size_t i) const { return data[i]; }
    float& operator[] (size_t i) { return data[i]; }

    Mat3 operator* (const Mat3& other) const {
        Mat3 result;
        for (size_t i = 0; i < 3; i++) {
            for (size_t j = 0; j < 3; j++) {
                result[i * 3 + j] = 0;
                for (size_t k = 0; k < 3; k++) {
                    result[i * 3 + j] += data[i * 3 + k] * other[k * 3 + j];
                }
            }
        }
        return result;
    }

    std::array<float, 3> operator* (const std::array<float, 3>& v) const {
        std::array<float, 3> result;
        for (size_t i = 0; i < 3; i++) {
            result[i] = 0;
            for (size_t j = 0; j < 3; j++) {
                result[i] += data[i * 3 + j] * v[j];
            }
        }
        return result;
    }

    Vec3 operator* (const Vec3& v) const {
        return Vec3(data[0] * v.x + data[3] * v.y + data[6] * v.z,
                    data[1] * v.x + data[4] * v.y + data[7] * v.z,
                    data[2] * v.x + data[5] * v.y + data[8] * v.z);
    }

    // gets the inverse of the matrix, or returns nullopt if the matrix is not invertible (or nearly so)
    // this is not a particularly good algorithm, but it works fine when performance and numerical precision aren't top priorities
    std::optional<Mat3> invert() const {
        Mat3 result;
        float invdet = (data[0] * (data[4] * data[8] - data[5] * data[7]) -
                        data[1] * (data[3] * data[8] - data[5] * data[6]) +
                        data[2] * (data[3] * data[7] - data[4] * data[6]));
        if (std::abs(invdet) < 1e-8f) {
            return std::nullopt;
        }
        float det = 1.0f / invdet;
        result[0] = (data[4] * data[8] - data[5] * data[7]) * det;
        result[1] = (data[2] * data[7] - data[1] * data[8]) * det;
        result[2] = (data[1] * data[5] - data[4] * data[2]) * det;
        result[3] = (data[5] * data[6] - data[3] * data[8]) * det;
        result[4] = (data[0] * data[8] - data[2] * data[6]) * det;
        result[5] = (data[2] * data[3] - data[0] * data[5]) * det;
        result[6] = (data[3] * data[7] - data[6] * data[4]) * det;
        result[7] = (data[1] * data[6] - data[0] * data[7]) * det;
        result[8] = (data[0] * data[4] - data[1] * data[3]) * det;
        return result;
    }

    static Mat3 identity() {
        Mat3 result{};
        result[0] = 1;
        result[4] = 1;
        result[8] = 1;
        return result;
    }

    static Mat3 diagonal(const std::array<float, 3>& v) {
        Mat3 result{};
        result[0] = v[0];
        result[4] = v[1];
        result[8] = v[2];
        return result;
    }
};

struct Mat4 {
    // data is stored in row-major order
    std::array<float, 16> data;

    float operator[] (size_t i) const { return data[i]; }
    float& operator[] (size_t i) { return data[i]; }

    Mat4 operator* (const Mat4& other) const {
        Mat4 result;
        for (size_t i = 0; i < 4; i++) {
            for (size_t j = 0; j < 4; j++) {
                result[i * 4 + j] = 0;
                for (size_t k = 0; k < 4; k++) {
                    result[i * 4 + j] += data[i * 4 + k] * other[k * 4 + j];
                }
            }
        }
        return result;
    }

    std::array<float, 4> operator* (const std::array<float, 4>& v) const {
        std::array<float, 4> result;
        for (size_t i = 0; i < 4; i++) {
            result[i] = 0;
            for (size_t j = 0; j < 4; j++) {
                result[i] += data[i * 4 + j] * v[j];
            }
        }
        return result;
    }

    Vec3 operator* (const Vec3& v) const {
        return Vec3(data[0] * v.x + data[4] * v.y + data[8] * v.z,
                     data[1] * v.x + data[5] * v.y + data[9] * v.z,
                     data[2] * v.x + data[6] * v.y + data[10] * v.z);
    }

    Pt3 operator* (const Pt3& v) const {
        return Pt3(data[0] * v.x + data[4] * v.y + data[8] * v.z + data[12],
                   data[1] * v.x + data[5] * v.y + data[9] * v.z + data[13],
                   data[2] * v.x + data[6] * v.y + data[10] * v.z + data[14]);
    }

    static Mat4 identity() {
        Mat4 result{};
        result[0] = 1;
        result[5] = 1;
        result[10] = 1;
        result[15] = 1;
        return result;
    }

    static Mat4 diagonal(const std::array<float, 4>& v) {
        Mat4 result{};
        result[0] = v[0];
        result[5] = v[1];
        result[10] = v[2];
        result[15] = v[3];
        return result;
    }

    static Mat4 rotation(const Vec3& axis, float angle) {
        Mat4 result{};
        result[0] = axis.x * axis.x + (1 - axis.x * axis.x) * std::cos(angle);
        result[1] = axis.x * axis.y * (1 - std::cos(angle)) - axis.z * std::sin(angle);
        result[2] = axis.x * axis.z * (1 - std::cos(angle)) + axis.y * std::sin(angle);
        result[4] = axis.x * axis.y * (1 - std::cos(angle)) + axis.z * std::sin(angle);
        result[5] = axis.y * axis.y + (1 - axis.y * axis.y) * std::cos(angle);
        result[6] = axis.y * axis.z * (1 - std::cos(angle)) - axis.x * std::sin(angle);
        result[8] = axis.x * axis.z * (1 - std::cos(angle)) - axis.y * std::sin(angle);
        result[9] = axis.y * axis.z * (1 - std::cos(angle)) + axis.x * std::sin(angle);
        result[10] = axis.z * axis.z + (1 - axis.z * axis.z) * std::cos(angle);
        result[15] = 1;
        return result;
    }
};

class Transform {
public:
    Transform(Mat4&& matrix, Mat4&& inverse_matrix) : m_mat(matrix), m_inv_mat(inverse_matrix) {}

    Vec3 apply(const Vec3& v) const {
        return m_mat * v;
    };
    Pt3 apply(const Pt3& p) const {
        return m_mat * p;
    };
    Vec3 apply_inverse(const Vec3& v) const {
        return m_inv_mat * v;
    };
    Pt3 apply_inverse(const Pt3& v) const {
        return m_inv_mat * v;
    };

    Vec3 operator* (const Vec3& v) const {
        return apply(v);
    }
    Pt3 operator* (const Pt3& p) const {
        return apply(p);
    }

    Transform operator* (const Transform& t) const {
        return Transform(m_mat * t.m_mat, t.m_inv_mat * m_inv_mat);
    }

    virtual ~Transform() {}

    static Transform identity() {
        return Transform(Mat4::identity(), Mat4::identity());
    }

    static Transform translation(const Vec3& v) {
        Mat4 mat = Mat4::identity();
        mat[12] = v.x;
        mat[13] = v.y;
        mat[14] = v.z;
        Mat4 inv_mat = Mat4::identity();
        inv_mat[12] = -v.x;
        inv_mat[13] = -v.y;
        inv_mat[14] = -v.z;
        return Transform(std::move(mat), std::move(inv_mat));
    }

    static Transform scale(const Vec3& v) {
        Mat4 mat = Mat4::diagonal({v.x, v.y, v.z, 1});
        Mat4 inv_mat = Mat4::diagonal({1 / v.x, 1 / v.y, 1 / v.z, 1});
        return Transform(std::move(mat), std::move(inv_mat));
    }

    static Transform rotation(const Vec3& axis, float angle) {
        return Transform(
            Mat4::rotation(axis, angle),
            Mat4::rotation(axis, -angle)
        );
    }

    Mat4 m_mat;
    Mat4 m_inv_mat;
};
