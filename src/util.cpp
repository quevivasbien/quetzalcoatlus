#include <algorithm>
#include <cmath>

#include "util.hpp"

float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

Vec3 spherical_direction(float sin_theta, float cos_theta, float phi) {
    return Vec3(
        std::clamp(sin_theta, -1.0f, 1.0f) * std::cos(phi),
        std::clamp(sin_theta, -1.0f, 1.0f) * std::sin(phi),
        std::clamp(cos_theta, -1.0f, 1.0f)
    );
}