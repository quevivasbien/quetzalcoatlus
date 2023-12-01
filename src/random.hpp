#pragma once

#include <random>

#include "vec.hpp"


class Sampler {
public:
    Sampler(
        uint32_t seed,
        uint32_t stratum_width    
    ) : rng(seed), dist(0.0f, 1.0f), stratum_width(stratum_width), permutation_seed(seed) {}

    float sample_1d();

    Vec2 sample_2d();

    void set_sample_index(uint32_t index);

    Vec3 sample_unit_sphere();

    Vec3 sample_within_unit_sphere() {
        return sample_unit_sphere() * sample_1d();
    }

    std::mt19937 rng;
    std::uniform_real_distribution<float> dist;
    uint32_t sample_index = 0;
    uint32_t depth = 0;
    uint32_t permutation_seed;
    uint32_t stratum_width;
};
