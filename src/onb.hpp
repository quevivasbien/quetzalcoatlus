#pragma once

#include <cmath>

#include "vec.hpp"

class OrthonormalBasis {
public:
    // construct an orthonormal basis from a single vector
    // n need not be pre-normalized
    // n will be the new z axis
    explicit OrthonormalBasis(Vec3 n) {
        n = n.normalized();
        float sign = n.z > 0.0f ? 1.0f : -1.0f;
        float a = -1.0f / (sign + n.z);
        float b = n.x * n.y * a;
        u[0] = Vec3(
            1.0f + sign * n.x * n.x * a,
            sign * b,
            -sign * n.x
        );
        u[1] = Vec3(
            b,
            sign + n.y * n.y * a,
            -n.y
        );
        u[2] = n;
    }

    Vec3 from_local(const Vec3& v) const {
        return u[0] * v.x + u[1] * v.y + u[2] * v.z;
    }

    Vec3 to_local(const Vec3& v) const {
        return Vec3(
            u[0].dot(v),
            u[1].dot(v),
            u[2].dot(v)
        );
    }

    Vec3 u[3];
};
