#pragma once

#include <optional>

#include "bounds.hpp"
#include "interval.hpp"
#include "material.hpp"
#include "ray.hpp"
#include "shape.hpp"


class Primitive {
public:
    virtual std::optional<Intersection> intersect(const Ray& ray, Interval t) const = 0;
    virtual Bounds bounds() const = 0;
};

// for use in debugging
class DummyPrimitive {
public:
    std::optional<Intersection> intersect(const Ray& ray, Interval t) const {
        return std::nullopt;
    }

    Bounds bounds() const {
        return Bounds::empty();
    }
};


class Aggregate : public Primitive {
public:
    explicit Aggregate(std::vector<Primitive*>&& primitives) : primitives(primitives) {
        for (const auto p : this->primitives) {
            m_bounds = m_bounds.union_with(p->bounds());
        }
    }

    std::optional<Intersection> intersect(const Ray& ray, Interval t) const override {
        std::optional<Intersection> isect = std::nullopt;
        for (const auto p : primitives) {
            std::optional<Intersection> i = p->intersect(ray, t);
            if (i) {
                isect.emplace(*i);
                t.high = isect->t;
            }
        }
        return isect;
    }

    Bounds bounds() const override {
        return m_bounds;
    }

    std::vector<Primitive*> primitives;
    Bounds m_bounds = Bounds::empty();
};



template <typename S, typename M>
class ShapePrimitive : public Primitive {
public:
    ShapePrimitive(const S& shape, const M& material) : shape(shape), material(material) {}

    std::optional<Intersection> intersect(const Ray& ray, Interval t) const override {
        std::optional<ShapeIntersection> si = shape.intersect(ray, t);
        if (si) {
            return Intersection(*si, this->material);
        }
        return std::nullopt;
    }

    Bounds bounds() const override {
        return shape.bounds();
    }

    S shape;
    M material;
};