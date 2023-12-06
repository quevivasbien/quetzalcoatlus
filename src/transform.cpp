#include <algorithm>

#include "transform.hpp"

Mat3 Mat3::operator* (const Mat3& other) const {
    Mat3 result{};
    for (size_t i = 0; i < 3; i++) {
        for (size_t j = 0; j < 3; j++) {
            for (size_t k = 0; k < 3; k++) {
                result[i * 3 + j] += data[i * 3 + k] * other[k * 3 + j];
            }
        }
    }
    return result;
}

std::array<float, 3> Mat3::operator* (const std::array<float, 3>& v) const {
    std::array<float, 3> result;
    for (size_t i = 0; i < 3; i++) {
        result[i] = 0;
        for (size_t j = 0; j < 3; j++) {
            result[i] += data[i * 3 + j] * v[j];
        }
    }
    return result;
}

Vec3 Mat3::operator* (const Vec3& v) const {
    return Vec3(data[0] * v.x + data[1] * v.y + data[2] * v.z,
                data[3] * v.x + data[4] * v.y + data[5] * v.z,
                data[6] * v.x + data[7] * v.y + data[8] * v.z);
}

// gets the inverse of the matrix, or returns nullopt if the matrix is not invertible (or nearly so)
std::optional<Mat3> Mat3::invert() const {
    // Matrix is
    // 0/a 1/d 2/g
    // 3/b 4/e 5/h
    // 6/c 7/f 8/i
    Mat3 adj({
        data[4] * data[8] - data[5] * data[7],
        data[2] * data[7] - data[1] * data[8],
        data[1] * data[5] - data[4] * data[2],
        data[5] * data[6] - data[3] * data[8],
        data[0] * data[8] - data[2] * data[6],
        data[2] * data[3] - data[0] * data[5],
        data[3] * data[7] - data[6] * data[4],
        data[1] * data[6] - data[0] * data[7],
        data[0] * data[4] - data[1] * data[3]
    });
    float det = data[0] * adj[0] + data[1] * adj[3] + data[2] * adj[6];
    if (det == 0.0f) {
        return std::nullopt;
    }
    float inv_det = 1.0f / det;

    std::transform(
        adj.data.begin(), adj.data.end(), adj.data.begin(),
        [&](float v) { return v * inv_det; }
    );

    // todo: properly handle nearly singular matrices

    return adj;
}

Mat3 Mat3::identity() {
    Mat3 result{};
    result[0] = 1;
    result[4] = 1;
    result[8] = 1;
    return result;
}

Mat3 Mat3::diagonal(const std::array<float, 3>& v) {
    Mat3 result{};
    result[0] = v[0];
    result[4] = v[1];
    result[8] = v[2];
    return result;
}

// todo: I think I messed up the mat4 and transform operations
// check and fix this

Mat4 Mat4::operator* (const Mat4& other) const {
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

std::array<float, 4> Mat4::operator* (const std::array<float, 4>& v) const {
    std::array<float, 4> result;
    for (size_t i = 0; i < 4; i++) {
        result[i] = 0;
        for (size_t j = 0; j < 4; j++) {
            result[i] += data[i * 4 + j] * v[j];
        }
    }
    return result;
}

Vec3 Mat4::operator* (const Vec3& v) const {
    return Vec3(data[0] * v.x + data[1] * v.y + data[2] * v.z,
                data[3] * v.x + data[4] * v.y + data[5] * v.z,
                data[6] * v.x + data[7] * v.y + data[8] * v.z);
}

Pt3 Mat4::operator* (const Pt3& v) const {
    return Pt3(data[0] * v.x + data[1] * v.y + data[2] * v.z + data[3],
               data[4] * v.x + data[5] * v.y + data[6] * v.z + data[7],
               data[8] * v.x + data[9] * v.y + data[10] * v.z + data[11]);
}

Mat4 Mat4::identity() {
    Mat4 result{};
    result[0] = 1;
    result[5] = 1;
    result[10] = 1;
    result[15] = 1;
    return result;
}

Mat4 Mat4::diagonal(const std::array<float, 4>& v) {
    Mat4 result{};
    result[0] = v[0];
    result[5] = v[1];
    result[10] = v[2];
    result[15] = v[3];
    return result;
}
Mat4 Mat4::rotation(const Vec3& axis, float angle) {
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

Vec3 Transform::apply(const Vec3& v) const {
    return m_mat * v;
};
Pt3 Transform::apply(const Pt3& p) const {
    return m_mat * p;
};
Vec3 Transform::apply_inverse(const Vec3& v) const {
    return m_inv_mat * v;
};
Pt3 Transform::apply_inverse(const Pt3& v) const {
    return m_inv_mat * v;
};

Vec3 Transform::operator* (const Vec3& v) const {
    return apply(v);
}
Pt3 Transform::operator* (const Pt3& p) const {
    return apply(p);
}

Transform Transform::operator* (const Transform& t) const {
    return Transform(m_mat * t.m_mat, t.m_inv_mat * m_inv_mat);
}

Transform Transform::identity() {
    return Transform(Mat4::identity(), Mat4::identity());
}

Transform Transform::translation(const Vec3& v) {
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

Transform Transform::scale(const Vec3& v) {
    Mat4 mat = Mat4::diagonal({v.x, v.y, v.z, 1});
    Mat4 inv_mat = Mat4::diagonal({1 / v.x, 1 / v.y, 1 / v.z, 1});
    return Transform(std::move(mat), std::move(inv_mat));
}

Transform Transform::rotation(const Vec3& axis, float angle) {
    return Transform(
        Mat4::rotation(axis, angle),
        Mat4::rotation(axis, -angle)
    );
}
