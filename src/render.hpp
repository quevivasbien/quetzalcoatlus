#pragma once

#include <mutex>

#include "camera.hpp"
#include "primitive.hpp"
#include "random.hpp"
#include "ray.hpp"
#include "image.hpp"
#include "vec.hpp"

Vec3 pixel_color(
    const Ray& r,
    const Primitive& world,
    size_t max_bounces,
    Sampler& sampler
);

void render_pixels(
    const Camera& camera,
    const Primitive* world,
    size_t n_samples,
    size_t max_bounces,
    float gamma,
    std::vector<float>& buffer,
    size_t start_index,
    size_t end_index,
    size_t& global_index,
    std::mutex& mutex
);

RenderResult render(
    const Camera& camera,
    const Primitive& world,
    size_t n_samples,
    size_t max_bounces,
    float gamma
);