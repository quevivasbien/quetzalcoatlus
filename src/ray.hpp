#pragma once

#include "vec.hpp"


class Ray {
public:
    Ray() : o(0.0f, 0.0f, 0.0f), d(0.0f, 0.0f, 0.0f) {}
    Ray(const Pt3& o, const Vec3& d) : o(o), d(d) {}
    
    Pt3 at(float t) const {
        return o + d * t;
    }

    Pt3 o;
    Vec3 d;
};
