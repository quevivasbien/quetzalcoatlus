#include "random.hpp"

float Sampler::sample_1d() {
    uint32_t stratum = (sample_index + depth * 2048 + permutation_seed * 1028) % stratum_width;
    depth++;
    float dx = dist(rng);
    return (float(stratum) + dx) / float(stratum_width);
}

Vec2 Sampler::sample_2d() {
    uint32_t stratum = (sample_index + depth * 2048 + permutation_seed * 1028) % (stratum_width * stratum_width);
    depth += 2;
    float x = float(stratum % stratum_width);
    float y = float(stratum / stratum_width);
    float dx = dist(rng);
    float dy = dist(rng);
    return Vec2(
        (x + dx) / float(stratum_width),
        (y + dy) / float(stratum_width)
    );
}

void Sampler::set_sample_index(uint32_t index) {
    sample_index = index;
    depth = 0;
}

Vec3 Sampler::sample_unit_sphere() {
    auto uv = sample_2d();

    float z = 1.0f - 2.0f * uv.x;
    float r = sqrtf(1.0f - z * z);
    float phi = 2.0f * M_PI * uv.y;

    return Vec3(r * cosf(phi), r * sinf(phi), z);
}