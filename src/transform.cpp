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
    std::array<float, 3> result{};
    for (size_t i = 0; i < 3; i++) {
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


Mat4 Mat4::operator* (const Mat4& other) const {
    Mat4 result {};
    for (size_t i = 0; i < 4; i++) {
        for (size_t j = 0; j < 4; j++) {
            for (size_t k = 0; k < 4; k++) {
                result[i * 4 + j] += data[i * 4 + k] * other[k * 4 + j];
            }
        }
    }
    return result;
}

std::array<float, 4> Mat4::operator* (const std::array<float, 4>& v) const {
    std::array<float, 4> result{};
    for (size_t i = 0; i < 4; i++) {
        for (size_t j = 0; j < 4; j++) {
            result[i] += data[i * 4 + j] * v[j];
        }
    }
    return result;
}

Vec3 Mat4::operator* (const Vec3& v) const {
    auto arr = operator*(v.to_homog());
    return Vec3::from_homog(arr);
}

Pt3 Mat4::operator* (const Pt3& v) const {
    auto arr = operator*(v.to_homog());
    return Pt3::from_homog(arr);
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

Mat4 Mat4::rotate_x(float angle) {
    Mat4 result{};
    result[0] = 1;
    result[5] = std::cos(angle);
    result[6] = -std::sin(angle);
    result[9] = std::sin(angle);
    result[10] = std::cos(angle);
    result[15] = 1;
    return result;
}

Mat4 Mat4::rotate_y(float angle) {
    Mat4 result{};
    result[0] = std::cos(angle);
    result[2] = std::sin(angle);
    result[5] = 1;
    result[8] = -std::sin(angle);
    result[10] = std::cos(angle);
    result[15] = 1;
    return result;
}

Mat4 Mat4::rotate_z(float angle) {
    Mat4 result{};
    result[0] = std::cos(angle);
    result[1] = -std::sin(angle);
    result[4] = std::sin(angle);
    result[5] = std::cos(angle);
    result[10] = 1;
    result[15] = 1;
    return result;
}

Mat4 Mat4::rotation(const Vec3& axis, float angle) {
    float ux = axis.x;
    float uy = axis.y;
    float uz = axis.z;
    float cos_theta = std::cos(angle);
    float sin_theta = std::sin(angle);
    return Mat4({
        cos_theta + ux * ux * (1 - cos_theta), ux * uy * (1 - cos_theta) - uz * sin_theta, ux * uz * (1 - cos_theta) + uy * sin_theta, 0,
        uy * ux * (1 - cos_theta) + uz * sin_theta, cos_theta + uy * uy * (1 - cos_theta), uy * uz * (1 - cos_theta) - ux * sin_theta, 0,
        uz * ux * (1 - cos_theta) - uy * sin_theta, uz * uy * (1 - cos_theta) + ux * sin_theta, cos_theta + uz * uz * (1 - cos_theta), 0,
        0, 0, 0, 1
    });
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

Ray Transform::apply(const Ray& r) const {
    return Ray(m_mat * r.o, m_mat * r.d);
}
Ray Transform::apply_inverse(const Ray& r) const {
    return Ray(m_inv_mat * r.o, m_inv_mat * r.d);
}

Transform Transform::apply(const Transform& t) const {
    return Transform(m_mat * t.m_mat, t.m_inv_mat * m_inv_mat);
}

Transform Transform::identity() {
    return Transform(Mat4::identity(), Mat4::identity());
}

Transform Transform::translation(const Vec3& v) {
    return translation(v.x, v.y, v.z);
}

Transform Transform::translation(float x, float y, float z) {
    Mat4 mat = Mat4::identity();
    mat[3] = x;
    mat[7] = y;
    mat[11] = z;
    Mat4 inv_mat = Mat4::identity();
    inv_mat[3] = -x;
    inv_mat[7] = -y;
    inv_mat[11] = -z;
    return Transform(std::move(mat), std::move(inv_mat));
}

Transform Transform::scale(const Vec3& v) {
    return scale(v.x, v.y, v.z);
}

Transform Transform::scale(float x, float y, float z) {
    Mat4 mat = Mat4::diagonal({x, y, z, 1});
    Mat4 inv_mat = Mat4::diagonal({1 / x, 1 / y, 1 / z, 1});
    return Transform(std::move(mat), std::move(inv_mat));
}

Transform Transform::scale(float c) {
    return scale(c, c, c);
}

Transform Transform::rotate_x(float angle) {
    return Transform(
        Mat4::rotate_x(angle),
        Mat4::rotate_x(-angle)
    );
}

Transform Transform::rotate_y(float angle) {
    return Transform(
        Mat4::rotate_y(angle),
        Mat4::rotate_y(-angle)
    );
}

Transform Transform::rotate_z(float angle) {
    return Transform(
        Mat4::rotate_z(angle),
        Mat4::rotate_z(-angle)
    );
}

Transform Transform::rotation(const Vec3& axis, float angle) {
    return Transform(
        Mat4::rotation(axis, angle),
        Mat4::rotation(axis, -angle)
    );
}
