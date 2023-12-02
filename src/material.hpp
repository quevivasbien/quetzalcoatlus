#pragma once

#include "random.hpp"
#include "ray.hpp"
#include "material.hpp"
#include "texture.hpp"
#include "vec.hpp"

class Material;

struct ShapeIntersection {
    float t;
    Vec2 uv;
    Vec3 normal;
    Pt3 point;
    bool outer_face;

    ShapeIntersection(float t, Vec2 uv, Vec3 normal, Pt3 point, bool outer_face) : t(t), uv(uv), normal(normal), point(point), outer_face(outer_face) {}
};


struct ScatterEvent {
    std::optional<Ray> new_ray;
    Vec3 color;

    ScatterEvent(std::optional<Ray> new_ray, Vec3 color) : new_ray(new_ray), color(color) {}
};


class Material {
public:
    virtual ScatterEvent scatter(const Ray& ray, const ShapeIntersection& isect, Sampler& sampler) const = 0;
};


template <typename T>
class LambertMaterial : public Material {
public:
    explicit LambertMaterial(T&& texture) : texture(texture) {}

    virtual ScatterEvent scatter(const Ray& ray, const ShapeIntersection& isect, Sampler& sampler) const override {
        Vec3 new_dir = isect.normal + sampler.sample_within_unit_sphere();

        if (new_dir.norm_squared() < 0.00001f) {
            return ScatterEvent(
                std::nullopt,
                texture.value(isect.uv, isect.point)
            );
        }

        Ray new_ray(isect.point, new_dir);
        return  ScatterEvent(
            std::make_optional(new_ray),
            texture.value(isect.uv, isect.point)
        );
    }

    T texture;
};


template <typename T>
class SpecularMaterial : public Material {
public:
    SpecularMaterial(T texture, float roughness) : texture(texture), roughness(roughness) {}

    virtual ScatterEvent scatter(const Ray& ray, const ShapeIntersection& isect, Sampler& sampler) const override {
        Vec3 new_dir = ray.d.reflect(isect.normal);
        if (roughness > 0.0f) {
            new_dir += roughness * sampler.sample_within_unit_sphere();
        }
        if (new_dir.dot(isect.normal) < 0.0f) {
            return ScatterEvent(
                std::nullopt,
                texture.value(isect.uv, isect.point)
            );
        }

        Ray new_ray(isect.point, new_dir);
        return ScatterEvent(
            std::make_optional(new_ray),
            texture.value(isect.uv, isect.point)
        );
    }

    T texture;
    float roughness;
};


template <typename T>
class RefractiveMaterial : public Material {
public:
    RefractiveMaterial(T texture, float ior) : texture(texture), ior(ior) {}

    virtual ScatterEvent scatter(const Ray& ray, const ShapeIntersection& isect, Sampler& sampler) const override {
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

        return  ScatterEvent(
            std::make_optional(Ray(isect.point, new_dir)),
            texture.value(isect.uv, isect.point)
        );

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

    virtual ScatterEvent scatter(const Ray& ray, const ShapeIntersection& isect, Sampler& sampler) const override {
        return ScatterEvent(
            std::nullopt,
            texture.value(isect.uv, isect.point)
        );
    }

    T texture;
};
