#pragma once

#include <optional>

#include "color/color.hpp"
#include "onb.hpp"
#include "random.hpp"
#include "vec.hpp"

struct BSDFSample {
    SpectrumSample spec;
    Vec3 wi;
    float pdf = 0.0f;
    float ior = 1.0f;
};


class BxDF {
public:
    virtual ~BxDF() {}

    // return the value of the distribution function for a pair of directions
    virtual SpectrumSample operator()(Vec3 wo, Vec3 wi) const = 0;

    // sample incident light direction for a given outgoing direction
    virtual std::optional<BSDFSample> sample(Vec3 wo, Sampler& sampler) const = 0;

    virtual float pdf(Vec3 wo, Vec3 wi, const Sampler& sampler) const = 0;

    // hemispherical-directional reflectance
    SpectrumSample reflectance(Vec3 wo, Sampler& sampler, size_t n_samples) const;

    // hemispherical-hemispherical reflectance
    SpectrumSample reflectance(Sampler& sampler, size_t n_samples) const;
};

// A wrapper around a BxDF that converts from world to local coordinates
class BSDF {
public:
    BSDF(Vec3 shading_normal, std::unique_ptr<BxDF>&& bxdf) : m_basis(shading_normal), m_bxdf(std::move(bxdf)) {}

    Vec3 local_from_render(Vec3 v) const { return m_basis.to_local(v); }
    
    Vec3 render_from_local(Vec3 v) const { return m_basis.from_local(v); }

    SpectrumSample operator()(Vec3 wo_render, Vec3 wi_render) const;

    std::optional<BSDFSample> sample(Vec3 wo_render, Sampler& sampler) const;

    float pdf(Vec3 wo_render, Vec3 wi_render, const Sampler& sampler) const;

    SpectrumSample reflectance(Vec3 wo_render, Sampler& sampler, size_t n_samples) const;

    SpectrumSample reflectance(Sampler& sampler, size_t n_samples) const;

    OrthonormalBasis m_basis;
    std::unique_ptr<BxDF> m_bxdf;
};


class DiffuseBxDF : public BxDF {
public:
    explicit DiffuseBxDF(float r) : m_reflectance(r) {}
    explicit DiffuseBxDF(const SpectrumSample& reflectance) : m_reflectance(reflectance) {}
    explicit DiffuseBxDF(SpectrumSample&& reflectance) : m_reflectance(std::move(reflectance)) {}

    SpectrumSample operator()(Vec3 wo, Vec3 wi) const override;
    std::optional<BSDFSample> sample(Vec3 wo, Sampler& sampler) const override;
    float pdf(Vec3 wo, Vec3 wi, const Sampler& sampler) const override;

private:
    SpectrumSample m_reflectance;
};


class ConductorBxDF : public BxDF {
public:
    ConductorBxDF(float ior, float absorption) : m_ior(ior), m_absorption(absorption) {}
    ConductorBxDF(const SpectrumSample& ior, const SpectrumSample& absorption) : m_ior(ior), m_absorption(absorption) {}

    SpectrumSample operator()(Vec3 wo, Vec3 wi) const override;
    std::optional<BSDFSample> sample(Vec3 wo, Sampler& sampler) const override;
    float pdf(Vec3 wo, Vec3 wi, const Sampler& sampler) const override;

private:
    SpectrumSample m_ior;
    SpectrumSample m_absorption;
};


class DielectricBxDF : public BxDF {
public:
    explicit DielectricBxDF(float ior) : m_ior(ior) {}

    SpectrumSample operator()(Vec3 wo, Vec3 wi) const override;
    std::optional<BSDFSample> sample(Vec3 wo, Sampler& sampler) const override;
    float pdf(Vec3 wo, Vec3 wi, const Sampler& sampler) const override;

private:
    float m_ior;
};


class ThinDielectricBxDF : public BxDF {
public:
    explicit ThinDielectricBxDF(float ior) : m_ior(ior) {}

    SpectrumSample operator()(Vec3 wo, Vec3 wi) const override;
    std::optional<BSDFSample> sample(Vec3 wo, Sampler& sampler) const override;
    float pdf(Vec3 wo, Vec3 wi, const Sampler& sampler) const override;
private:
    float m_ior;
};