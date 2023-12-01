#pragma once

#include <optional>

#include "bounds.hpp"
#include "interval.hpp"
#include "ray.hpp"
#include "vec.hpp"

struct ShapeIntersection {
    float t;
    Vec2 uv;
    Vec3 normal;
    Pt3 point;
    bool outer_face;

    ShapeIntersection(float t, Vec2 uv, Vec3 normal, Pt3 point, bool outer_face) : t(t), uv(uv), normal(normal), point(point), outer_face(outer_face) {}
};

class Shape {
public:
    virtual std::optional<ShapeIntersection> intersect(const Ray& ray, Interval t) const = 0;
    virtual Bounds bounds() const = 0;
};


class Sphere : public Shape {
public:
    Sphere(Pt3 center, float radius) : center(center), radius(radius) {}

    virtual std::optional<ShapeIntersection> intersect(const Ray& ray, Interval t) const override;

    virtual Bounds bounds() const override;

    Pt3 center;
    float radius;

private:
    Vec2 get_uv(Vec3 normal, bool outer_face) const;
};
