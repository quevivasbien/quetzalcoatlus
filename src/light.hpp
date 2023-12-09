#pragma once

#include "vec.hpp"
#include "color/spectrum_sample.hpp"

struct SurfaceInteraction;

struct Light {

    virtual SpectrumSample emission(const SurfaceInteraction& si, const Vec3& wo, const WavelengthSample& wavelengths) const = 0;
};


struct SimpleLight : public Light {
    std::shared_ptr<const Spectrum> m_radiance;

    explicit SimpleLight(std::shared_ptr<const Spectrum> radiance) : m_radiance(radiance) {}

    SpectrumSample emission(const SurfaceInteraction& si, const Vec3& wo, const WavelengthSample& wavelengths) const override;
};


struct DirectionalLight : public Light {
    std::shared_ptr<const Spectrum> m_radiance;

    explicit DirectionalLight(std::shared_ptr<const Spectrum> radiance) : m_radiance(radiance) {}

    SpectrumSample emission(const SurfaceInteraction& si, const Vec3& wo, const WavelengthSample& wavelengths) const override;
};