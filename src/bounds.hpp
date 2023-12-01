#pragma once

#include <cfloat>
#include <cmath>

#include "vec.hpp"


class Bounds {
public:
    Pt3 min;
    Pt3 max;

    // copy constructor
    Bounds(const Bounds &b) : min(b.min), max(b.max) {}

    Bounds(Pt3 min, Pt3 max) {
        this->min = Pt3(fminf(min.x, max.x), fminf(min.y, max.y), fminf(min.z, max.z));
        this->max = Pt3(fmaxf(min.x, max.x), fmaxf(min.y, max.y), fmaxf(min.z, max.z));
    }

    static Bounds empty() {
        return Bounds(Pt3(FLT_MAX, FLT_MAX, FLT_MAX), Pt3(FLT_MIN, FLT_MIN, FLT_MIN));
    }

    Bounds union_with(const Bounds &b) {
        return Bounds(
            Pt3(fminf(min.x, b.min.x), fminf(min.y, b.min.y), fminf(min.z, b.min.z)),
            Pt3(fmaxf(max.x, b.max.x), fmaxf(max.y, b.max.y), fmaxf(max.z, b.max.z))
        );
    }
};