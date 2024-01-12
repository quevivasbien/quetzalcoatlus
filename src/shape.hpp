#pragma once

#include <cmath>
#include <tuple>

#include "sampler.hpp"
#include "vec.hpp"

enum ShapeType {
    SPHERE,
    TRIANGLE,
    QUAD,
    OBJ
};

struct ShapeSample {
    Pt3 p;
    Vec3 normal;
    float pdf;
};

// currently used just to sample points on a shape
class Shape {
public:
    // randomly sample a point on the shape's surface
    virtual ShapeSample sample_point(Vec2 sample2) const = 0;
    // get pdf for sampled point on the shape's surface
    virtual float area() const = 0;
    virtual float pdf(const Pt3& p) const {
        return 1.0f / area();
    }
    virtual ShapeType type() const = 0;
};


// A 2d parallelogram
// The points of the parallelogram are p00, p00 + du, p00 + du + dv, p00 + dv
// The normal points in the direction of du x dv
class Quad : public Shape {
public:
    Quad(const Pt3& p00, const Vec3& du, const Vec3& dv)
        : m_p00(p00), m_du(du), m_dv(dv), m_normal(du.cross(dv).normalized()), m_area(du.cross(dv).norm()) {}

    ShapeSample sample_point(Vec2 sample2) const override {
        Pt3 p = m_p00 + m_du * sample2.x + m_dv * sample2.y;
        return { p, m_normal, 1.0f / area() };
    }

    float area() const override {
        return m_area;
    }

    float pdf(const Pt3& p) const override {
        //todo!
        return 1.0f / area();
    }

    ShapeType type() const override {
        return QUAD;
    }

    std::tuple<Pt3, Pt3, Pt3, Pt3> get_vertices() const {
        return { m_p00, m_p00 + m_du, m_p00 + m_du + m_dv, m_p00 + m_dv };
    }

private:
    Vec3 m_normal;
    Pt3 m_p00;
    Vec3 m_du;
    Vec3 m_dv;
    float m_area;
};

class Sphere : public Shape {
public:
    Sphere(const Pt3& center, float radius) : m_center(center), m_radius(radius) {}

    ShapeSample sample_point(Vec2 sample2) const override {
        Vec3 n = Sampler::sample_uniform_sphere(sample2);
        auto p = m_center + m_radius * n;
        return { p, n, 1.0f / area() };
    }

    float area() const override {
        return 4.0f * M_PI * m_radius * m_radius;
    }

    float pdf(const Pt3& p) const override {
        return 1.0f / area();
    }

    ShapeType type() const override {
        return SPHERE;
    }

    Pt3 m_center;
    float m_radius;
};