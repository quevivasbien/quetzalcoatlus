#pragma once

#include <mutex>

#include "camera.hpp"
#include "primitive.hpp"
#include "random.hpp"
#include "ray.hpp"
#include "image.hpp"
#include "vec.hpp"

RenderResult render(
    const Camera& camera,
    const Primitive& world,
    size_t n_samples,
    size_t max_bounces
);