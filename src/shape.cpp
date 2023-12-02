#include "shape.hpp"

std::optional<ShapeIntersection> Sphere::intersect(const Ray& ray, Interval t) const {
    Vec3 oc = ray.o - center;
    float a = ray.d.norm_squared();
    float b = oc.dot(ray.d);
    float c = oc.norm_squared() - radius * radius;
    float discriminant = b * b - a * c;
    if (discriminant < 0.0f) {
        return std::nullopt;
    }
    float sqrt_d = sqrtf(discriminant);
    float root = (-b - sqrt_d) / a;
    if (!t.contains(root)) {
        root = (-b + sqrt_d) / a;
        if (!t.contains(root)) {
            return std::nullopt;
        }
    }
    
    Pt3 point = ray.at(root);
    Vec3 normal = (point - center) / radius;
    bool outer_face = normal.dot(ray.d) < 0.0f;
    Vec2 uv = get_uv(normal, outer_face);

    return ShapeIntersection(
        root,
        uv,
        normal,
        point,
        outer_face
    );
}

Bounds Sphere::bounds() const {
    return Bounds(
        center - Vec3(radius, radius, radius),
        center + Vec3(radius, radius, radius)
    );
}

Vec2 Sphere::get_uv(Vec3 normal, bool outer_face) const {
    if (!outer_face) {
        normal = -normal;
    }
    float phi = atan2f(normal.z, normal.x) + M_PI;
    float theta = acosf(normal.y);
    return Vec2(phi / (2.0f * M_PI), theta / M_PI);
}


std::optional<ShapeIntersection> Plane::intersect(const Ray& ray, Interval t) const {
    float denom = normal.dot(ray.d);
    if (denom > 0.0f) {
        return std::nullopt;
    }
    float dist = (m_d - normal.dot(ray.o)) / denom;
    if (!t.contains(dist)) {
        return std::nullopt;
    }
    Pt3 p = ray.at(dist);
    return ShapeIntersection(
        dist,
        Vec2(),
        normal,
        p,
        ray.d.dot(normal) < 0.0f
    );
}

Bounds Plane::bounds() const {
    return Bounds::infinite();
}


std::optional <ShapeIntersection> Quad::intersect(const Ray& ray, Interval t) const {
    auto si = plane.intersect(ray, t);
    if (!si) {
        return std::nullopt;
    }
    Vec3 plane_delta = si->point - plane.point;
    float a = w.dot(plane_delta.cross(v));
    float b = w.dot(u.cross(plane_delta));
    if (a < 0.0f || b < 0.0f || a > 1.0f || b > 1.0f) {
        return std::nullopt;
    }
    si->uv = Vec2(a, b);
    return si;
}

Bounds Quad::bounds() const {
    return m_bounds;
}
