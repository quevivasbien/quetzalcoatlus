#pragma once

#include "color/spectrum_sample.hpp"
#include "vec.hpp"
#include "shape.hpp"
#include "transform.hpp"

struct SurfaceInteraction;

struct LightSample {
    SpectrumSample spec;
    Vec3 wi;
    float pdf;
    Pt3 p_light;
};

enum LightType {
    POINT,
    DIRECTIONAL,
    AREA
};

class Light {
public:
    virtual ~Light() {}

    Light(const Transform& render_from_light, std::shared_ptr<const Spectrum> spectrum, LightType type) : m_spectrum(spectrum), m_type(type), m_render_from_light(render_from_light) {}
    
    virtual SpectrumSample total_emission(const WavelengthSample& wavelengths) const = 0;

    // sample light received at point on surface (si refers to surface receiving light, not the light itself)
    virtual std::optional<LightSample> sample(const SurfaceInteraction& si, const WavelengthSample& wavelengths, Sampler& sampler) const = 0;
    // get pdf for light from source, along wi to point p
    // note that here p is the point that receives the light, not a point on the light
    virtual float pdf(const Pt3& p, const Vec3& wi) const {
        return 0.0f;
    };
    // get light emitted in a given direction; only valid for area lights
    // here p is the point *on the light* and n is the surface normal at that point
    // w is the direction in which the light is emitted
    virtual SpectrumSample emission(const Pt3& p, const Vec3& n, const Vec3& w, const WavelengthSample& wavelengths) const {
        return SpectrumSample(0.0f);
    }

    LightType type() const {
        return m_type;
    }

protected:
    std::shared_ptr<const Spectrum> m_spectrum;
    LightType m_type;
    Transform m_render_from_light;
};


class PointLight : public Light {
public:
    PointLight(const Transform& render_from_light, std::shared_ptr<const Spectrum> spectrum, float scale = 1.0f) : Light(render_from_light, spectrum, POINT), m_scale(scale) {}

    SpectrumSample total_emission(const WavelengthSample& wavelengths) const override;

    std::optional<LightSample> sample(const SurfaceInteraction& si, const WavelengthSample& wavelengths, Sampler& sampler) const override;

private:
    float m_scale;
};


class AreaLight : public Light {
public:
    AreaLight(
        const Transform& render_from_light,
        std::unique_ptr<Shape>&& shape,
        std::shared_ptr<const Spectrum> spectrum,
        float scale = 1.0f,
        bool two_sided = false
    ) : Light(render_from_light, spectrum, AREA), m_shape(std::move(shape)), m_scale(scale), m_two_sided(two_sided) {
        m_area = m_shape->area();
    }

    SpectrumSample total_emission(const WavelengthSample& wavelengths) const override;

    std::optional<LightSample> sample(const SurfaceInteraction& si, const WavelengthSample& wavelengths, Sampler& sampler) const override;
    float pdf(const Pt3& p, const Vec3& wi) const override;
    SpectrumSample emission(const Pt3& p, const Vec3& n, const Vec3& w, const WavelengthSample& wavelengths) const override;

private:
    std::unique_ptr<Shape> m_shape;
    float m_scale;
    bool m_two_sided;
    float m_area;
};