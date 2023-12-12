#pragma once

#include <cmath>
#include <tuple>

#include "random.hpp"
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
    virtual ShapeSample sample_point(Sampler& sampler) const = 0;
    // get pdf for sampled point on the shape's surface
    virtual float pdf(const Pt3& p) const = 0;
    virtual float area() const = 0;
    virtual ShapeType type() const = 0;
};


class Quad : public Shape {
public:
    Quad(const Pt3& p00, const Pt3& p10, const Pt3& p11, const Pt3& p01)
        : m_p00(p00), m_p10(p10), m_p11(p11), m_p01(p01), m_normal((p10 - p00).cross(p01 - p00).normalize()) {
            if (p00 == p10 || p10 == p11 || p11 == p01 || p01 == p00) {
                return;
            }
            // check if vertices are coplanar
            if ((std::abs(m_normal.dot((p11 - p00).normalize()))) > 1e-5f) {
                return;
            }
            // check if planar vertices form a rectangle by checking if vertices are equidistant from center
            Pt3 center = Pt3(p00 + p10 + p11 + p01 / 4.0f);
            std::array<float, 4> distances = {
                (p00 - center).norm_squared(),
                (p10 - center).norm_squared(),
                (p11 - center).norm_squared(),
                (p01 - center).norm_squared()
            };
            for (int i = 1; i < 3; i++) {
                if (std::abs(distances[i] - distances[0]) / distances[0] > 1e-4f) {
                    return;
                }
            }
            m_is_rectangle = true;
        }

    Pt3 point_from_uv(const Vec2& uv) const {
        return Pt3((1.0f - uv.y) * ((1.0f - uv.x) * m_p00 + uv.x * m_p10) + uv.y * ((1.0f - uv.x) * m_p01 + uv.x * m_p11));
    }

    ShapeSample sample_point(Sampler& sampler) const override {
        float pdf_ = 1.0f;
        Vec2 uv;
        if (m_is_rectangle) {
            uv = sampler.sample_2d();
        }
        else {
            // sample with appropriate uniform area sampling
            std::array<float, 4> w = {
                (m_p10 - m_p00).cross(m_p01 - m_p00).norm(),
                (m_p10 - m_p00).cross(m_p11 - m_p10).norm(),
                (m_p01 - m_p00).cross(m_p11 - m_p01).norm(),
                (m_p11 - m_p10).cross(m_p11 - m_p01).norm()
            };
            uv = sampler.sample_bilinear(w);
            pdf_ = sampler.bilinear_pdf(uv, w);
        }
        Vec3 pu0 = m_p00 * (1.0f - uv.y) + m_p01 * uv.y;
        Vec3 pu1 = m_p10 * (1.0f - uv.y) + m_p11 * uv.y;
        Pt3 p = Pt3(pu0 * (1.0f - uv.x) + pu1 * uv.x);
        return { p, m_normal, pdf_ };
    }

    float area() const override {
        return 0.5f * ((m_p10 - m_p00).cross(m_p01 - m_p00).norm() + (m_p01 - m_p11).cross(m_p10 - m_p11).norm());
    }

    float pdf(const Pt3& p) const override {
        //todo!
        return 0;
    }

    ShapeType type() const override {
        return QUAD;
    }

    std::tuple<Pt3, Pt3, Pt3, Pt3> get_vertices() const {
        return { m_p00, m_p10, m_p11, m_p01 };
    }

private:
    Vec3 m_normal;
    Pt3 m_p00;
    Pt3 m_p10;
    Pt3 m_p11;
    Pt3 m_p01;

    bool m_is_rectangle = false;
};

class Sphere : public Shape {
public:
    Sphere(const Pt3& center, float radius) : m_center(center), m_radius(radius) {}

    ShapeSample sample_point(Sampler& sampler) const override {
        Vec3 n = sampler.sample_uniform_sphere();
        auto p = m_center + m_radius * n;
        return { p, n, sampler.uniform_sphere_pdf() };
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