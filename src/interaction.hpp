#pragma once

#include <optional>

#include "bxdf.hpp"
#include "light.hpp"
#include "material.hpp"
#include "scattering/medium.hpp"
#include "vec.hpp"

struct SurfaceInteraction {

    SurfaceInteraction(
        const Pt3& point,
        const Vec3& wo,
        const Vec3& normal,
        const Vec2& uv,
        const Material* material = nullptr,
        const AreaLight* light = nullptr,
        const std::optional<MediumInterface>& medium_interface = std::nullopt
    ) : point(point), wo(wo), normal(normal), uv(uv), material(material), light(light), medium_interface(medium_interface) {}

    std::optional<BSDF> bsdf(const Ray& ray, WavelengthSample& wavelengths, float sample) const {
        if (!material) {
            return std::nullopt;
        }
        return material->bsdf(*this, wavelengths, sample);
    }

    SpectrumSample emission(const Vec3 w, const WavelengthSample& wavelengths) const {
        if (!light) {
            return SpectrumSample(0.0f);
        }
        return light->emission(this->point, this->normal, w, wavelengths);
    }

    Ray skip_intersection(const Ray& ray) const {
        return Ray(point, ray.d);
    }

    // returns the medium that the unit vector w is coming from / going into
    const Medium* get_medium(Vec3 w) const {
        if (medium_interface) {
            return w.dot(normal) > 0.0f ? medium_interface->outside : medium_interface->inside;
        }
        return nullptr;
    }

    const Medium* get_medium() const {
        if (medium_interface) {
            return medium_interface->inside;
        }
        return nullptr;
    }

    Pt3 point;
    Vec3 wo;
    Vec3 normal;
    Vec2 uv;
    const Material* material = nullptr;
    const Light* light = nullptr;
    std::optional<MediumInterface> medium_interface = std::nullopt;
};