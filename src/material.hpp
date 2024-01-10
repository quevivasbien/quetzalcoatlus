#pragma once

#include <algorithm>
#include <cassert>

#include "bxdf.hpp"
#include "color/color.hpp"
#include "sampler.hpp"
#include "ray.hpp"
#include "material.hpp"
#include "texture.hpp"
#include "vec.hpp"

struct SurfaceInteraction;

class Material {
public:
    virtual BSDF bsdf(const SurfaceInteraction& si, WavelengthSample& wavelengths, float sample) const = 0;
};


class DiffuseMaterial : public Material {
public:
    explicit DiffuseMaterial(std::unique_ptr<Texture>&& texture) : m_texture(std::move(texture)) {}

    template <typename T>
    explicit DiffuseMaterial(T&& texture) : m_texture(std::make_unique<T>(std::forward<T>(texture))) {}

    BSDF bsdf(const SurfaceInteraction& si, WavelengthSample& wavelengths, float sample) const override;

    std::unique_ptr<Texture> m_texture;
};


class ConductiveMaterial : public Material {
public:
    ConductiveMaterial(float ior, float absorption) : m_ior(std::make_shared<ConstantSpectrum>(ior)), m_absorption(std::make_shared<ConstantSpectrum>(absorption)), m_roughness(0.0f, 0.0f) {}

    ConductiveMaterial(
        std::shared_ptr<const Spectrum> ior,
        std::shared_ptr<const Spectrum> absorption,
        TrowbridgeReitzDistribution roughness = TrowbridgeReitzDistribution(0.0f, 0.0f)
    ) : m_ior(ior), m_absorption(absorption), m_roughness(roughness) {}

    BSDF bsdf(const SurfaceInteraction& si, WavelengthSample& wavelengths, float sample) const override;

    static ConductiveMaterial alluminum(float roughness_a = 0.0f, float roughness_b = 0.0f);
    static ConductiveMaterial copper(float roughness_a = 0.0f, float roughness_b = 0.0f);

    std::shared_ptr<const Spectrum> m_ior;
    std::shared_ptr<const Spectrum> m_absorption;
    TrowbridgeReitzDistribution m_roughness;
};


class DielectricMaterial : public Material {
public:
    explicit DielectricMaterial(float ior) : m_ior(std::make_shared<ConstantSpectrum>(ior)), is_constant(true) {}

    explicit DielectricMaterial(std::shared_ptr<const Spectrum> ior) : m_ior(ior), is_constant(false) {}

    BSDF bsdf(const SurfaceInteraction& si, WavelengthSample& wavelengths, float sample) const override;

    bool is_constant;
    std::shared_ptr<const Spectrum> m_ior;
};


class ThinDielectricMaterial : public Material {
public:
    explicit ThinDielectricMaterial(float ior) : m_ior(std::make_shared<ConstantSpectrum>(ior)), is_constant(true) {}
    
    explicit ThinDielectricMaterial(std::shared_ptr<const Spectrum> ior) : m_ior(ior), is_constant(false) {}

    BSDF bsdf(const SurfaceInteraction& si, WavelengthSample& wavelengths, float sample) const override;

    bool is_constant;
    std::shared_ptr<const Spectrum> m_ior;
};


template <size_t N>
class MixedMaterial : public Material {
public:
    explicit MixedMaterial(std::array<std::unique_ptr<Material>, N>&& materials, std::array<float, N>&& weights) : m_materials(std::move(materials)), m_weights(std::move(weights)) {
        float weight_sum = std::accumulate(m_weights.begin(), m_weights.end(), 0.0f);
        assert(weight_sum > 0.0f);
        // normalize weights to sum to 1
        std::transform(m_weights.begin(), m_weights.end(), m_weights.begin(), [weight_sum](float w) {
            return w / weight_sum;
        });
    }
    
    BSDF bsdf(const SurfaceInteraction& si, WavelengthSample& wavelengths, float sample) const override {
        size_t idx = sample * m_materials.size();
        return m_materials[idx]->bsdf(si, wavelengths, sample);
    }

    std::array<std::unique_ptr<Material>, N> m_materials;
    std::array<float, N> m_weights;
};
