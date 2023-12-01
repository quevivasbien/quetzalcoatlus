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
    virtual std::optional<ShapeIntersection> intersect(const Ray& ray, Interval t) const = 0;
    virtual Bounds bounds() const = 0;
};


class Sphere : public Shape {
public:
    Sphere(Pt3 center, float radius) : center(center), radius(radius) {}

    virtual std::optional<ShapeIntersection> intersect(const Ray& ray, Interval t) const override {
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

    virtual Bounds bounds() const override {
        return Bounds(
            center - Vec3(radius, radius, radius),
            center + Vec3(radius, radius, radius)
        );
    }

    Pt3 center;
    float radius;

private:
    Vec2 get_uv(Vec3 normal, bool outer_face) const {
        if (!outer_face) {
            normal = -normal;
        }
        float phi = atan2f(normal.z, normal.x) + M_PI;
        float theta = acosf(normal.y);
        return Vec2(phi / (2.0f * M_PI), theta / M_PI);
    }
};

