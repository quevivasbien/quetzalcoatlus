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

Vec2 Sampler::sample_uniform_disk() {
    Vec2 uv(dist(rng), dist(rng));
    Vec2 offset = 2.0f * uv - Vec2(1.0f, 1.0f);
    if (offset.x == 0.0f && offset.y == 0.0f) {
        return Vec2(0.0f, 0.0f);
    }
    float theta, r;
    if (fabsf(offset.x) > fabsf(offset.y)) {
        r = offset.x;
        theta = M_PI_4 * (offset.y / offset.x);
    }
    else {
        r = offset.y;
        theta = M_PI_2 - M_PI_4 * (offset.x / offset.y);
    }
    return Vec2(r * cosf(theta), r * sinf(theta));
}

Vec3 Sampler::sample_uniform_hemisphere() {
    Vec2 uv(dist(rng), dist(rng));
    float z = uv.x;
    float r = sqrtf(1.0f - z * z);
    float phi = 2.0f * M_PI * uv.y;
    return Vec3(
        r * cosf(phi),
        r * sinf(phi),
        z
    );
}

Vec3 Sampler::sample_uniform_sphere() {
    Vec2 uv(dist(rng), dist(rng));

    float z = 1.0f - 2.0f * uv.x;
    float r = sqrtf(1.0f - z * z);
    float phi = 2.0f * M_PI * uv.y;

    return Vec3(r * cosf(phi), r * sinf(phi), z);
}

Vec3 Sampler::sample_cosine_hemisphere() {
    // sample from unit disk
    Vec2 d = sample_uniform_disk();
    // project up to a hemisphere
    float z = fsqrt(1.0f - d.x * d.x - d.y * d.y);
    return Vec3(d.x, d.y, z);
}