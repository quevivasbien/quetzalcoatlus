#pragma once

#include "bxdf.hpp"
#include "color/color.hpp"
#include "random.hpp"
#include "ray.hpp"
#include "material.hpp"
#include "texture.hpp"
#include "vec.hpp"

struct SurfaceInteraction;

class Material {
public:
    virtual BSDF bsdf(const SurfaceInteraction& si, WavelengthSample& wavelengths) const = 0;
};


class DiffuseMaterial : public Material {
public:
    explicit DiffuseMaterial(std::unique_ptr<Texture>&& texture) : m_texture(std::move(texture)) {}

    template <typename T>
    explicit DiffuseMaterial(T&& texture) : m_texture(std::make_unique<T>(std::forward<T>(texture))) {}

    BSDF bsdf(const SurfaceInteraction& si, WavelengthSample& wavelengths) const override;

    std::unique_ptr<Texture> m_texture;
};


class ConductiveMaterial : public Material {
public:
    ConductiveMaterial(float ior, float absorption) : m_ior(std::make_shared<ConstantSpectrum>(ior)), m_absorption(std::make_shared<ConstantSpectrum>(absorption)) {}

    ConductiveMaterial(std::shared_ptr<Spectrum> ior, std::shared_ptr<Spectrum> absorption) : m_ior(ior), m_absorption(absorption) {}

    BSDF bsdf(const SurfaceInteraction& si, WavelengthSample& wavelengths) const override;

    std::shared_ptr<Spectrum> m_ior;
    std::shared_ptr<Spectrum> m_absorption;
};


class DielectricMaterial : public Material {
public:
    explicit DielectricMaterial(float ior) : m_ior(std::make_shared<ConstantSpectrum>(ior)), is_constant(true) {}

    explicit DielectricMaterial(std::shared_ptr<Spectrum> ior) : m_ior(ior), is_constant(false) {}

    BSDF bsdf(const SurfaceInteraction& si, WavelengthSample& wavelengths) const override;

    bool is_constant;
    std::shared_ptr<Spectrum> m_ior;
};