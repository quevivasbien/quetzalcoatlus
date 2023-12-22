#pragma once

#include <optional>

#include "color/color.hpp"
#include "onb.hpp"
#include "sampler.hpp"
#include "vec.hpp"

struct ScatterType {
    bool specular = false;
    bool transmission = false;
};

struct BSDFSample {
    SpectrumSample spec;
    Vec3 wi;
    float pdf = 0.0f;
    float ior = 1.0f;
    bool pdf_is_proportional = false;
    ScatterType scatter_type;
};


class BxDF {
public:
    virtual ~BxDF() {}

    // return the value of the distribution function for a pair of directions
    virtual SpectrumSample operator()(Vec3 wo, Vec3 wi) const = 0;

    // sample incident light direction for a given outgoing direction
    virtual std::optional<BSDFSample> sample(Vec3 wo, float sample1, Vec2 sample2) const = 0;

    virtual float pdf(Vec3 wo, Vec3 wi) const = 0;

    // hemispherical-directional reflectance
    SpectrumSample reflectance(Vec3 wo, Sampler& sampler, size_t n_samples) const;

    // hemispherical-hemispherical reflectance
    SpectrumSample reflectance(Sampler& sampler, size_t n_samples) const;

    virtual bool is_specular() const { return false; }
};

// A wrapper around a BxDF that converts from world to local coordinates
class BSDF {
public:
    BSDF(Vec3 shading_normal, std::unique_ptr<BxDF>&& bxdf) : m_basis(shading_normal), m_bxdf(std::move(bxdf)) {}

    Vec3 local_from_render(Vec3 v) const { return m_basis.to_local(v); }
    
    Vec3 render_from_local(Vec3 v) const { return m_basis.from_local(v); }

    SpectrumSample operator()(Vec3 wo_render, Vec3 wi_render) const;

    std::optional<BSDFSample> sample(Vec3 wo_render, float sample1, Vec2 sample2) const;

    float pdf(Vec3 wo_render, Vec3 wi_render) const;

    SpectrumSample reflectance(Vec3 wo_render, Sampler& sampler, size_t n_samples) const;

    SpectrumSample reflectance(Sampler& sampler, size_t n_samples) const;

    bool is_specular() const { return m_bxdf->is_specular(); }

    OrthonormalBasis m_basis;
    std::unique_ptr<BxDF> m_bxdf;
};


class DiffuseBxDF : public BxDF {
public:
    explicit DiffuseBxDF(float r) : m_reflectance(r) {}
    explicit DiffuseBxDF(const SpectrumSample& reflectance) : m_reflectance(reflectance) {}
    explicit DiffuseBxDF(SpectrumSample&& reflectance) : m_reflectance(std::move(reflectance)) {}

    SpectrumSample operator()(Vec3 wo, Vec3 wi) const override;
    std::optional<BSDFSample> sample(Vec3 wo, float sample1, Vec2 sample2) const override;
    float pdf(Vec3 wo, Vec3 wi) const override;

private:
    SpectrumSample m_reflectance;
};

// represents microgeometry (roughness) of a surface
struct TrowbridgeReitzDistribution {
    TrowbridgeReitzDistribution(float alpha_x, float alpha_y) : m_alpha_x(alpha_x), m_alpha_y(alpha_y) {}

    float operator()(Vec3 wm) const;
    float operator()(Vec3 w, Vec3 wm) const;
    
    Vec3 sample(Vec3 w, Vec2 sample2) const;

    bool is_smooth() const;

    float m_alpha_x;
    float m_alpha_y;

    float masking(Vec3 w) const;
    float masking_shadowing(Vec3 wo, Vec3 wi) const;

private:
    float lambda(Vec3 w) const;
};

class ConductorBxDF : public BxDF {
public:
    ConductorBxDF(float ior, float absorption, float roughness = 0.0f) : m_ior(ior), m_absorption(absorption), m_roughness(roughness, roughness) {}
    ConductorBxDF(
        const SpectrumSample& ior,
        const SpectrumSample& absorption,
        const TrowbridgeReitzDistribution& roughness = TrowbridgeReitzDistribution(0.0f, 0.0f)
    ) : m_ior(ior), m_absorption(absorption), m_roughness(roughness) {}

    SpectrumSample operator()(Vec3 wo, Vec3 wi) const override;
    std::optional<BSDFSample> sample(Vec3 wo, float sample1, Vec2 sample2) const override;
    float pdf(Vec3 wo, Vec3 wi) const override;

    bool is_specular() const override;

private:
    SpectrumSample m_ior;
    SpectrumSample m_absorption;
    TrowbridgeReitzDistribution m_roughness;
};


class DielectricBxDF : public BxDF {
public:
    explicit DielectricBxDF(float ior) : m_ior(ior) {}

    SpectrumSample operator()(Vec3 wo, Vec3 wi) const override;
    std::optional<BSDFSample> sample(Vec3 wo, float sample1, Vec2 sample2) const override;
    float pdf(Vec3 wo, Vec3 wi) const override;

    bool is_specular() const override { return true; }

private:
    float m_ior;
};


class ThinDielectricBxDF : public BxDF {
public:
    explicit ThinDielectricBxDF(float ior) : m_ior(ior) {}

    SpectrumSample operator()(Vec3 wo, Vec3 wi) const override;
    std::optional<BSDFSample> sample(Vec3 wo, float sample1, Vec2 sample2) const override;
    float pdf(Vec3 wo, Vec3 wir) const override;

    bool is_specular() const override { return true; }

private:
    float m_ior;
};