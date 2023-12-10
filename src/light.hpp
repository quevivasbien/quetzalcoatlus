#pragma once

#include "color/spectrum_sample.hpp"
#include "vec.hpp"
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

    virtual SpectrumSample emission(const SurfaceInteraction& si, const Vec3& wo, const WavelengthSample& wavelengths) const = 0;
    virtual std::optional<LightSample> sample(const SurfaceInteraction& si, const WavelengthSample& wavelengths, Sampler& sampler) const = 0;
    virtual float pdf(const SurfaceInteraction& si, const Vec3& wi) const = 0;

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

    SpectrumSample emission(const SurfaceInteraction& si, const Vec3& wo, const WavelengthSample& wavelengths) const override;
    std::optional<LightSample> sample(const SurfaceInteraction& si, const WavelengthSample& wavelengths, Sampler& sampler) const override;
    float pdf(const SurfaceInteraction& si, const Vec3& wi) const override;

private:
    float m_scale;
};
