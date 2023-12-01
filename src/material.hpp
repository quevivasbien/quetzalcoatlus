#pragma once

#include "random.hpp"
#include "ray.hpp"
#include "material.hpp"
#include "shape.hpp"
#include "texture.hpp"
#include "vec.hpp"

class Material;

struct Intersection : ShapeIntersection {
    const Material& material;

    Intersection(
        const ShapeIntersection& si,
        const Material& material
    ) : ShapeIntersection(si), material(material) {}
};


struct ScatterEvent {
    std::optional<Ray> new_ray;
    Vec3 color;

    ScatterEvent(std::optional<Ray> new_ray, Vec3 color) : new_ray(new_ray), color(color) {}
};

class Material {
public:
    virtual std::optional<ScatterEvent> scatter(const Ray& ray, const Intersection& isect, Sampler& sampler) const = 0;
};


template <typename T>
class LambertMaterial : public Material {
public:
    explicit LambertMaterial(T&& texture) : texture(texture) {}

    virtual std::optional<ScatterEvent> scatter(const Ray& ray, const Intersection& isect, Sampler& sampler) const override {
        Vec3 new_dir = isect.normal + sampler.sample_within_unit_sphere();

        if (new_dir.norm_squared() < 0.00001f) {
            return std::nullopt;
        }

        Ray new_ray(isect.point, new_dir);
        return std::make_optional(ScatterEvent(
            std::make_optional(new_ray),
            texture.value(isect.uv, isect.point)
        ));
    }

    T texture;
};


template <typename T>
class EmissiveMaterial : public Material {
public:
    explicit EmissiveMaterial(T texture) : texture(texture) {}

    virtual std::optional<ScatterEvent> scatter(const Ray& ray, const Intersection& isect, Sampler& sampler) const override {
        return std::make_optional(ScatterEvent(
            std::nullopt,
            texture.value(isect.uv, isect.point)
        ));
    }

    T texture;
};
