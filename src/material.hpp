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
    Vec3 normal;

    ScatterEvent(std::optional<Ray> new_ray, Vec3 color, Vec3 normal) : new_ray(new_ray), color(color), normal(normal) {}
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
            texture.value(isect.uv, isect.point),
            isect.normal
        ));
    }

    T texture;
};


template <typename T>
class SpecularMaterial : public Material {
public:
    SpecularMaterial(T texture, float roughness) : texture(texture), roughness(roughness) {}

    virtual std::optional<ScatterEvent> scatter(const Ray& ray, const Intersection& isect, Sampler& sampler) const override {
        Vec3 new_dir = ray.d.reflect(isect.normal);
        if (roughness > 0.0f) {
            new_dir += roughness * sampler.sample_within_unit_sphere();
        }
        if (new_dir.dot(isect.normal) < 0.0f) {
            return std::nullopt;
        }

        Ray new_ray(isect.point, new_dir);
        return std::make_optional(ScatterEvent(
            std::make_optional(new_ray),
            texture.value(isect.uv, isect.point),
            isect.normal
        ));
    }

    T texture;
    float roughness;
};


template <typename T>
class RefractiveMaterial : public Material {
public:
    RefractiveMaterial(T texture, float ior) : texture(texture), ior(ior) {}

    virtual std::optional<ScatterEvent> scatter(const Ray& ray, const Intersection& isect, Sampler& sampler) const override {
        float ior_ratio;
        Vec3 normal;
        if (isect.outer_face) {
            ior_ratio = 1.0f / this->ior;
            normal = isect.normal;
        }
        else {
            ior_ratio = this->ior;
            normal = -isect.normal;
        }

        float cos_theta = -normal.dot(ray.d);
        Vec3 new_dir;
        if (reflectance(cos_theta, ior_ratio) > sampler.sample_1d()) {
            new_dir = ray.d.reflect(normal);
        }
        else {
            new_dir = ray.d.normalize().refract(normal, ior_ratio);
        }

        return std::make_optional(ScatterEvent(
            std::make_optional(Ray(isect.point, new_dir)),
            texture.value(isect.uv, isect.point),
            isect.normal
        ));

    }

    T texture;
    float ior;

private:
    float reflectance(float cos_theta, float ior_ratio) const {
        float r0 = (1.0f - ior_ratio) / (1.0f + ior_ratio);
        r0 = r0 * r0;
        return r0 + (1.0f - r0) * powf(1.0f - cos_theta, 5.0f);
    }
};


template <typename T>
class EmissiveMaterial : public Material {
public:
    explicit EmissiveMaterial(T texture) : texture(texture) {}

    virtual std::optional<ScatterEvent> scatter(const Ray& ray, const Intersection& isect, Sampler& sampler) const override {
        return std::make_optional(ScatterEvent(
            std::nullopt,
            texture.value(isect.uv, isect.point),
            isect.normal
        ));
    }

    T texture;
};
