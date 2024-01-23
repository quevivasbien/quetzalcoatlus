#pragma once

#include "vec.hpp"

const float ONE_MINUS_EPS = float(0x1.fffffep-1);

float lerp(float a, float b, float t);

Vec3 spherical_direction(float sin_theta, float cos_theta, float phi);