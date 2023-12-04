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

    Vec2 sample_uniform_disk();
    Vec3 sample_uniform_hemisphere();
    float uniform_hemisphere_pdf() {
        return 0.5 * M_1_PI;
    }
    Vec3 sample_uniform_sphere();
    float uniform_sphere_pdf() {
        return 0.25 * M_1_PI;
    }
    Vec3 sample_cosine_hemisphere();
    float cosine_hemisphere_pdf(float cos_theta) {
        return cos_theta * M_1_PI;
    }

    std::mt19937 rng;
    std::uniform_real_distribution<float> dist;
    uint32_t sample_index = 0;
    uint32_t depth = 0;
    uint32_t permutation_seed;
    uint32_t stratum_width;
};
