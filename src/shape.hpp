#include <cmath>

#include "random.hpp"
#include "vec.hpp"

struct ShapeSample {
    Pt3 p;
    Vec3 normal;
    float pdf;
};

// currently used just to sample points on a shape
class Shape {
public:
    virtual ShapeSample sample_point(Sampler& sampler) const = 0;
    virtual float area() const = 0;
};


class Quad : public Shape {
public:
    Quad(const Pt3& p0, const Pt3& p1, const Pt3& p2, const Pt3& p3)
        : m_p0(p0), m_p1(p1), m_p2(p2), m_p3(p3), m_normal((p1 - p0).cross(p3 - p0).normalize()) {}

    Pt3 point_from_uv(const Vec2& uv) const {
        return Pt3((1.0f - uv.y) * ((1.0f - uv.x) * m_p0 + uv.x * m_p1) + uv.y * ((1.0f - uv.x) * m_p3 + uv.x * m_p2));
    }

    ShapeSample sample_point(Sampler& sampler) const override {
        // todo: fix this (make properly uniform) for non-square quads
        return { point_from_uv(sampler.sample_2d()), m_normal, 1.0f };
    }

    float area() const override {
        return 0.5f * ((m_p1 - m_p0).cross(m_p3 - m_p0).norm() + (m_p3 - m_p2).cross(m_p1 - m_p2).norm());
    }

private:
    Vec3 m_normal;
    Pt3 m_p0;
    Pt3 m_p1;
    Pt3 m_p2;
    Pt3 m_p3;
};

class Sphere : public Shape {
public:
    explicit Sphere(float radius, Pt3 center = Pt3(0.0f, 0.0f, 0.0f)) : m_center(center), m_radius(radius) {}

    ShapeSample sample_point(Sampler& sampler) const override {
        Vec3 n = sampler.sample_uniform_sphere();
        auto p = m_center + m_radius * n;
        return { p, n, sampler.uniform_sphere_pdf() };
    }

    float area() const override {
        return 4.0f * M_PI * m_radius * m_radius;
    }

    Pt3 m_center;
    float m_radius;
};