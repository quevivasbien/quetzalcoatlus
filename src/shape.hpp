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


class Plane : public Shape {
public:
    Plane(Pt3 point, Vec3 normal) : point(point), normal(normal), m_d(point.dot(normal)) {}

    virtual std::optional<ShapeIntersection> intersect(const Ray& ray, Interval t) const override;

    virtual Bounds bounds() const override;

    Pt3 point;
    Vec3 normal;
    float m_d;
};


class Quad : public Shape {
public:
    Quad(Pt3 point, Vec3 u, Vec3 v) : plane(point, u.cross(v).normalize()), u(u), v(v), m_bounds(point, point + u + v) {
        Vec3 n = u.cross(v);
        w = n / n.norm_squared();
    }

    virtual std::optional<ShapeIntersection> intersect(const Ray& ray, Interval t) const override;

    virtual Bounds bounds() const override;

    Plane plane;
    Vec3 u;
    Vec3 v;
    Vec3 w;
    Bounds m_bounds;
};
