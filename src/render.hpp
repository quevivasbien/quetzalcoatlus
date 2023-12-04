#pragma once

#include "camera.hpp"
#include "scene.hpp"
#include "image.hpp"
#include "vec.hpp"

RenderResult render(
    const Camera& camera,
    const Scene& world,
    size_t n_samples,
    size_t max_bounces
);