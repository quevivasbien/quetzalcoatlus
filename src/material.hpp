#pragma once

#include "bxdf.hpp"
#include "color/color.hpp"
#include "random.hpp"
#include "ray.hpp"
#include "material.hpp"
#include "texture.hpp"
#include "vec.hpp"

class Material;

struct ShapeIntersection {
    Vec2 uv;
    Vec3 normal;
    Pt3 point;
    bool outer_face;
    WavelengthSample wavelengths;
};


struct ScatterEvent {
    std::optional<Ray> new_ray;
    SpectrumSample color;
    float pdf = 1.0f;
};


class Material {
public:
    virtual ScatterEvent scatter(const Ray& ray, const ShapeIntersection& isect, Sampler& sampler) const = 0;
};


template <typename T>
class LambertMaterial : public Material {
public:
    explicit LambertMaterial(T&& texture) : m_texture(texture) {}

    virtual ScatterEvent scatter(const Ray& ray, const ShapeIntersection& isect, Sampler& sampler) const override {
        auto r = m_texture.value(isect.uv, isect.point, isect.wavelengths);
        BSDF bsdf(
            isect.normal,
            std::make_unique<DiffuseBxDF>(std::move(r))
        );
        auto sample = bsdf.sample(-ray.d, sampler);
        if (!sample) {
            return ScatterEvent {
                .new_ray = std::nullopt,
                .color = SpectrumSample(0.0f),
            };
        }
        return ScatterEvent {
            .new_ray = Ray(isect.point, sample->wi),
            .color = sample->spec,
            .pdf = sample->pdf
        };
    }

    T m_texture;
};


template <typename T>
class SpecularMaterial : public Material {
public:
    SpecularMaterial(T texture, float roughness) : texture(texture), roughness(roughness) {}

    virtual ScatterEvent scatter(const Ray& ray, const ShapeIntersection& isect, Sampler& sampler) const override {
        Vec3 new_dir = ray.d.reflect(isect.normal);
        if (roughness > 0.0f) {
            new_dir += roughness * sampler.sample_uniform_sphere() * sampler.sample_1d();
        }

        return ScatterEvent(
            Ray(isect.point, new_dir),
            texture.value(isect.uv, isect.point, isect.wavelengths)
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

        // TODO: update this to refract different wavelengths differently

        return ScatterEvent(
            Ray(isect.point, new_dir),
            texture.value(isect.uv, isect.point, isect.wavelengths)
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
        if (isect.outer_face) {
            return ScatterEvent(
                std::nullopt,
                texture.value(isect.uv, isect.point, isect.wavelengths)
            );
        }
        else {
            return ScatterEvent(
                std::nullopt,
                SpectrumSample(0.0f)
            );
        }
    }

    T texture;
};
