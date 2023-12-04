#include "camera.hpp"

Camera::Camera(
    size_t image_width,
    size_t image_height,
    float fov,
    const Transform& transform
) : image_width(image_width), image_height(image_height) {
    float viewport_height = 2.0f * tanf(fov * 0.5f);
    float viewport_width = viewport_height * float(image_width) / float(image_height);

    pos = transform * Pt3(0.0f, 0.0f, 0.0f);
    look_at = transform * Vec3(0.0f, 0.0f, -1.0f);
    up = transform * Vec3(0.0f, 1.0f, 0.0f);
    right = look_at.cross(up);

    Vec3 viewport_u = viewport_width * right;
    Vec3 viewport_v = viewport_height * up;

    pixel_delta_u = viewport_u / float(image_width);
    pixel_delta_v = viewport_v / float(image_height);

    viewport_bottom_left = pos + look_at - viewport_u * 0.5f - viewport_v * 0.5f;
}

Ray Camera::cast_ray(float u, float v) const {
    return Ray(
        pos,
        viewport_bottom_left + pixel_delta_u * u + pixel_delta_v * v - pos
    );
}