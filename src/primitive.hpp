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