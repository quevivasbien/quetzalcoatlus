#pragma once

#include <random>

#include "vec.hpp"


uint32_t permutation_element(uint32_t i, uint32_t l, uint32_t p) {
    uint32_t w = l - 1;
    w |= w >> 1;
    w |= w >> 2;
    w |= w >> 4;
    w |= w >> 8;
    w |= w >> 16;
    do {
        i ^= p;
        i *= 0xe170893d;
        i ^= p >> 16;
        i ^= (i & w) >> 4;
        i ^= p >> 8;
        i *= 0x0929eb3f;
        i ^= p >> 23;
        i ^= (i & w) >> 1;
        i *= 1 | p >> 27;
        i *= 0x6935fa69;
        i ^= (i & w) >> 11;
        i *= 0x74dcb303;
        i ^= (i & w) >> 2;
        i *= 0x9e501cc3;
        i ^= (i & w) >> 2;
        i *= 0xc860a3df;
        i &= w;
        i ^= i >> 5;
    } while (i >= l);
    return (i + p) % l;
}

class Sampler {
public:
    Sampler(
        uint32_t seed,
        uint32_t stratum_width    
    ) : rng(seed), dist(0.0f, 1.0f), stratum_width(stratum_width), permutation_seed(seed) {}

    float sample_1d() {
        uint32_t index = sample_index + depth * 2048;
        depth++;
        uint32_t stratum = permutation_element(index, stratum_width, permutation_seed);
        float dx = dist(rng);
        return (float(stratum) + dx) / float(stratum_width);
    }

    Vec2 sample_2d() {
        uint32_t index = sample_index + depth * 2048;
        depth += 2;
        uint32_t stratum = permutation_element(index, stratum_width * stratum_width, permutation_seed);
        float x = float(stratum % stratum_width);
        float y = float(stratum / stratum_width);
        float dx = dist(rng);
        float dy = dist(rng);
        return Vec2(
            (x + dx) / float(stratum_width),
            (y + dy) / float(stratum_width)
        );
    }

    void set_sample_index(uint32_t index) {
        sample_index = index;
        depth = 0;
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


    std::mt19937 rng;
    std::uniform_real_distribution<float> dist;
    uint32_t sample_index = 0;
    uint32_t depth = 0;
    uint32_t permutation_seed;
    uint32_t stratum_width;
};
