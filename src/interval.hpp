#pragma once

struct Interval {
    float low;
    float high;

    Interval(float low, float high) : low(low), high(high) {}

    bool contains(float t) const {
        return t >= low && t <= high;
    }
};