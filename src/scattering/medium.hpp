#pragma once

#include "color/color.hpp"
#include "phase_function.hpp"
#include "vec.hpp"

class Medium;

struct MediumSample {
    SpectrumSample absorption;
    SpectrumSample scattering;
    PhaseFunction phase;
};

struct MediumInterface {
    const Medium* inside;
    const Medium* outside;
};

// Represents the maximum absorption + scattering of a medium through a segment of a ray's path
struct RayMajorantSegment {
    SpectrumSample value;
    float t_min;
    float t_max;
};

class RayMajorantIterator {
public:
    virtual std::optional<RayMajorantSegment> next() = 0;
};

class HomogeneousMajorantIterator : public RayMajorantIterator {
public:
    HomogeneousMajorantIterator() : m_segment(std::nullopt) {}

    HomogeneousMajorantIterator(
        SpectrumSample&& value,
        float t_min, float t_max
    ) : m_segment(RayMajorantSegment { std::move(value), t_min, t_max }) {}

    std::optional<RayMajorantSegment> next() override;

private:
    std::optional<RayMajorantSegment> m_segment;
};

class Medium {
public:
    virtual MediumSample sample_point(Pt3 p, const WavelengthSample& wavelengths) const = 0;
    virtual std::unique_ptr<RayMajorantIterator> sample_ray(Ray ray, float t_max, const WavelengthSample& wavelengths) const = 0;
};

class HomogeneousMedium : public Medium {
public:
    HomogeneousMedium(
        std::shared_ptr<const Spectrum> absorption,
        std::shared_ptr<const Spectrum> scattering,
        const PhaseFunction& phase
    ) : m_absorption(absorption), m_scattering(scattering), phase(phase) {}

    HomogeneousMedium(
        std::shared_ptr<const Spectrum> absorption,
        std::shared_ptr<const Spectrum> scattering,
        float g
    ) : m_absorption(absorption), m_scattering(scattering), phase(g) {}

    MediumSample sample_point(Pt3 p, const WavelengthSample& wavelengths) const override;
    std::unique_ptr<RayMajorantIterator> sample_ray(Ray ray, float t_max, const WavelengthSample& wavelengths) const override;

private:
    std::shared_ptr<const Spectrum> m_absorption;
    std::shared_ptr<const Spectrum> m_scattering;

    PhaseFunction phase;
};