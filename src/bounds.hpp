#pragma once

#include <cfloat>

#include "vec.hpp"


struct Bounds {
    Pt3 min;
    Pt3 max;

    Bounds(Pt3 min, Pt3 max) {
        this->min = Pt3(fminf(min.x, max.x), fminf(min.y, max.y), fminf(min.z, max.z));
        this->max = Pt3(fmaxf(min.x, max.x), fmaxf(min.y, max.y), fmaxf(min.z, max.z));
    }

    static Bounds empty() {
        return Bounds(Pt3(FLT_MAX, FLT_MAX, FLT_MAX), Pt3(FLT_MIN, FLT_MIN, FLT_MIN));
    }
};