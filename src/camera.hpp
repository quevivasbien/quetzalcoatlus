#pragma once

#include <cstddef>
#include <cmath>

#include "color/color.hpp"
#include "vec.hpp"
#include "ray.hpp"
#include "transform.hpp"

class Camera {
public:
    Camera(
        size_t image_width,
        size_t image_height,
        float fov,
        const Transform& transform = Transform::identity(),
        PixelSensor&& sensor = PixelSensor::CANON_EOS(),
        const Medium *medium = nullptr
    );

    // creates a ray pointing to the pixel coordinates (u, v)
    Ray cast_ray(float u, float v) const;

    size_t image_height;
    size_t image_width;
    Pt3 pos;
    Vec3 look_at;
    Vec3 up;
    Vec3 right;
    Vec3 viewport_bottom_left;
    Vec3 pixel_delta_u;
    Vec3 pixel_delta_v;

    PixelSensor sensor;

    const Medium *medium;
};