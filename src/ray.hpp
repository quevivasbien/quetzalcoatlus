#pragma once

#include "vec.hpp"

class Medium;

class Ray {
public:
    Ray() : o(0.0f, 0.0f, 0.0f), d(0.0f, 0.0f, 0.0f), medium(nullptr) {}
    Ray(const Pt3& o, const Vec3& d, const Medium* medium = nullptr) : o(o), d(d), medium(medium) {}
    
    Pt3 at(float t) const {
        return o + d * t;
    }

    Pt3 o;
    Vec3 d;
    const Medium* medium;
};
