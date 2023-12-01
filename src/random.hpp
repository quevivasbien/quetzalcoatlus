#pragma once

#include <random>

#include "vec.hpp"


class Sampler {
public:
    std::mt19937 rng;
    std::uniform_real_distribution<float> dist;

    explicit Sampler(uint32_t seed) : rng(seed), dist(0.0f, 1.0f) {}

    float sample_1d() {
        // TODO: implement deterministic sampling and stratified sampling
        return dist(rng);
    }

    Vec2 sample_2d() {
        // TODO: implement deterministic sampling and stratified sampling
        return Vec2(dist(rng), dist(rng));
    }

    Vec3 sample_unit_sphere() {
        auto uv = sample_2d();

        float z = 1.0f - 2.0f * uv.x;
        float r = sqrtf(1.0f - z * z);
        float phi = 2.0f * M_PI * uv.y;

        return Vec3(r * cosf(phi), r * sinf(phi), z);
    }

    Vec3 sample_within_unit_sphere() {
        return sample_unit_sphere() * sample_1d();
    }
};
