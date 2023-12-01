#pragma once

#include "vec.hpp"


class Ray {
public:
    Ray(const Pt3& o, const Vec3& d) : o(o), d(d) {}
    
    Pt3 at(float t) const {
        return o + d * t;
    }

    Pt3 o;
    Vec3 d;
};
