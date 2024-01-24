#pragma once

#include <iostream>
#include <random>

#include "color/color.hpp"
#include "phase_function.hpp"
#include "ray.hpp"
#include "sampler.hpp"
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


template <typename F>
SpectrumSample sample_majorant(
    Ray ray,
    float t_max,
    float u,
    std::mt19937& rng,
    const WavelengthSample& wavelengths,
    F callback
) {
    t_max *= ray.d.norm();
    ray.d = ray.d.normalized();

    if (!ray.medium) {
        std::cerr << "Called sample_majorant with no medium" << std::endl;
        return SpectrumSample(0.0f);
    }
    auto iter = ray.medium->sample_ray(ray, t_max, wavelengths);

    SpectrumSample maj(1.0f);
    bool done = false;
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    while (!done) {
        auto seg_opt = iter->next();
        if (!seg_opt) {
            return maj;
        }
        auto seg = *seg_opt;

        if (seg.value[0] == 0.0f) {
            // todo: what is the point of this?
            float dt = seg.t_max - seg.t_min;
            if (std::isinf(dt)) {
                dt = std::numeric_limits<float>::max();
            }
            auto exp = seg.value.map([dt](float v) { return std::exp(-dt * v); });
            maj *= exp;
            continue;
        }

        while (true) {
            float t = seg.t_min + Sampler::sample_exponential(u, seg.value[0]);
            u = dist(rng);
            if (t < seg.t_max) {
                // t is within the segment
                maj *= seg.value.map([t, &seg](float v) { return std::exp(-(t - seg.t_min) * v); });
                MediumSample ms = ray.medium->sample_point(ray.at(t), wavelengths);
                
                if (!callback(ray.at(t), ms, seg.value, maj)) {
                    done = true;
                    break;
                }
            }
            else {
                // t is past the segment
                float dt = seg.t_max - seg.t_min;
                if (std::isinf(dt)) {
                    dt = std::numeric_limits<float>::max();
                }
                maj *= seg.value.map([dt](float v) { return std::exp(-dt * v); });
                break;
            }
        }
    }
    return SpectrumSample(1.0f);
}