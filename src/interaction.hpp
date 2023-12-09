#pragma once

#include "bxdf.hpp"
#include "light.hpp"
#include "material.hpp"
#include "vec.hpp"

struct Interaction {
    Pt3 point;
    Vec3 wo;
    Vec3 normal;
    Vec2 uv;
};

struct SurfaceInteraction : public Interaction {
    SurfaceInteraction(
        const Pt3& point,
        const Vec3& wo,
        const Vec3& normal,
        const Vec2& uv,
        const Material* material,
        const Light* light
    ) : Interaction { point, wo, normal, uv }, material(material), light(light) {}

    std::optional<BSDF> bsdf(const Ray& ray, WavelengthSample& wavelengths, Sampler& sampler) const {
        if (!material) {
            return std::nullopt;
        }
        return material->bsdf(*this, wavelengths);
    }

    SpectrumSample emission(const Vec3 wo, const WavelengthSample& wavelengths) const {
        if (!light) {
            return SpectrumSample(0.0f);
        }
        return light->emission(*this, wo, wavelengths);
    }

    Ray skip_intersection(const Ray& ray) const {
        return Ray(point, ray.d);
    }

    const Material* material;
    const Light* light;
};