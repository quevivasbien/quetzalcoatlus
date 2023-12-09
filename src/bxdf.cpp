#include <algorithm>
#include <cmath>
#include <complex>

#include "bxdf.hpp"

// utility functions for directions in hemispherical coordinate system
float cos_theta(Vec3 w) {
    return w.z;
}

float cos2_theta(Vec3 w) {
    return w.z * w.z;
}

float abs_cos_theta(Vec3 w) {
    return std::abs(w.z);
}

float sin2_theta(Vec3 w) {
    return std::max(0.0f, 1.0f - cos2_theta(w));
}

float sin_theta(Vec3 w) {
    return std::sqrt(sin2_theta(w));
}

float tan_theta(Vec3 w) {
    return sin_theta(w) / cos_theta(w);
}

float tan2_theta(Vec3 w) {
    return sin2_theta(w) / cos2_theta(w);
}


SpectrumSample BxDF::reflectance(Vec3 wo, Sampler& sampler, size_t n_samples) const {
    SpectrumSample r(0.0f);
    for (size_t i = 0; i < n_samples; i++) {
        auto bs = sample(wo, sampler);
        if (bs) {
            r += bs->spec * abs_cos_theta(bs->wi) / bs->pdf;
        }
    }
    return r / float(n_samples);
}

SpectrumSample BxDF::reflectance(Sampler& sampler, size_t n_samples) const {
    SpectrumSample r(0.0f);
    for (size_t i = 0; i < n_samples; i++) {
        Vec3 wo = sampler.sample_uniform_hemisphere();
        if (wo.z == 0.0f) {
            continue;
        }
        float pdf_o = sampler.uniform_hemisphere_pdf();
        auto bs = sample(wo, sampler);
        if (bs) {
            r += bs->spec * abs_cos_theta(bs->wi) * abs_cos_theta(wo) / (pdf_o * bs->pdf);
        }
    }
    return r / (M_PI * n_samples);
}


SpectrumSample BSDF::operator()(Vec3 wo_render, Vec3 wi_render) const {
    Vec3 wi = local_from_render(wi_render);
    Vec3 wo = local_from_render(wo_render);
    if (wo.z == 0.0f) {
        return SpectrumSample(0.0f);
    }
    return (*m_bxdf)(wo, wi);
}

std::optional<BSDFSample> BSDF::sample(Vec3 wo_render, Sampler& sampler) const {
    Vec3 wo = local_from_render(wo_render);
    if (wo.z == 0.0f) {
        return std::nullopt;
    }
    auto bs = (*m_bxdf).sample(wo, sampler);
    if (!bs || bs->spec.is_zero() || bs->pdf == 0.0f || bs->wi.z == 0.0f) {
        return std::nullopt;
    }
    bs->wi = render_from_local(bs->wi);
    return *bs;
}

float BSDF::pdf(Vec3 wo_render, Vec3 wi_render, const Sampler& sampler) const {
    Vec3 wi = local_from_render(wi_render);
    Vec3 wo = local_from_render(wo_render);
    if (wo.z == 0.0f) {
        return 0.0f;
    }
    return m_bxdf->pdf(wo, wi, sampler);
}

SpectrumSample BSDF::reflectance(Vec3 wo_render, Sampler& sampler, size_t n_samples) const {
    return reflectance(local_from_render(wo_render), sampler, n_samples);
}

SpectrumSample BSDF::reflectance(Sampler& sampler, size_t n_samples) const {
    return reflectance(sampler, n_samples);
}


SpectrumSample DiffuseBxDF::operator()(Vec3 wo, Vec3 wi) const {
    if (wo.z * wi.z <= 0.0f) {
        return SpectrumSample(0.0f);
    }
    return m_reflectance * M_1_PI;
}

std::optional<BSDFSample> DiffuseBxDF::sample(Vec3 wo, Sampler& sampler) const {
    Vec3 wi = sampler.sample_cosine_hemisphere();
    if (wo.z < 0.0f) {
        wi.z *= -1.0f;
    }
    float pdf_ = sampler.cosine_hemisphere_pdf(abs_cos_theta(wi));
    return BSDFSample {
        .spec = m_reflectance * M_1_PI,
        .wi = wi,
        .pdf = pdf_
    };
}

float DiffuseBxDF::pdf(Vec3 wo, Vec3 wi, const Sampler& sampler) const {
    if (wo.z * wi.z <= 0.0f) {
        return 0.0f;
    }
    return sampler.cosine_hemisphere_pdf(abs_cos_theta(wi));
}


Vec3 reflect(Vec3 wo, Vec3 n) {
    return -wo + 2.0f * wo.dot(n) * n;
}

std::optional<std::pair<Vec3, float>> refract(Vec3 wi, Vec3 n, float ior) {
    float cos_theta_i = wi.dot(n);
    // flip interface orientation if cos is negative
    if (cos_theta_i < 0.0f) {
        ior = 1.0f / ior;
        cos_theta_i = -cos_theta_i;
        n = -n;
    }
    // use snell's law to compute direction of transmission
    float sin2_theta_i = std::max(0.0f, 1.0f - cos_theta_i * cos_theta_i);
    float sin2_theta_t = sin2_theta_i / (ior * ior);
    // handle case of total internal reflection
    if (sin2_theta_t >= 1.0f) {
        return std::nullopt;
    }

    float cos_theta_t = std::sqrt(1.0f - sin2_theta_t);

    Vec3 wt = -wi / ior + (cos_theta_i / ior - cos_theta_t) * n;

    return std::make_pair(wt, ior);
}

float dielectric_reflectance(float cos_theta_i, float ior) {
    cos_theta_i = std::clamp(cos_theta_i, -1.0f, 1.0f);
    if (cos_theta_i < 0.0f) {
        ior = 1.0f / ior;
        cos_theta_i = -cos_theta_i;
    }
    float sin2_theta_i = 1.0f - cos_theta_i * cos_theta_i;
    float sin2_theta_t = sin2_theta_i / (ior * ior);
    if (sin2_theta_t >= 1.0f) {
        return 1.0f;
    }
    float cos_theta_t = std::sqrt(1.0f - sin2_theta_t);
    float r_parallel = (ior * cos_theta_i - cos_theta_t) / (ior * cos_theta_i + cos_theta_t);
    float r_perp = (cos_theta_i - ior * cos_theta_t) / (cos_theta_i + ior * cos_theta_t);
    return 0.5f * (r_parallel * r_parallel + r_perp * r_perp);
}

float dielectric_reflectance(float cos_theta_i, std::complex<float> ior) {
    cos_theta_i = std::clamp(cos_theta_i, -1.0f, 1.0f);
    if (cos_theta_i < 0.0f) {
        ior = 1.0f / ior;
        cos_theta_i = -cos_theta_i;
    }
    float sin2_theta_i = 1.0f - cos_theta_i * cos_theta_i;
    std::complex sin2_theta_t = sin2_theta_i / (ior * ior);
    std::complex cos_theta_t = std::sqrt(1.0f - sin2_theta_t);
    std::complex r_parallel = (ior * cos_theta_i - cos_theta_t) / (ior * cos_theta_i + cos_theta_t);
    std::complex r_perp = (cos_theta_i - ior * cos_theta_t) / (cos_theta_i + ior * cos_theta_t);
    return 0.5f * (std::norm(r_parallel) + std::norm(r_perp));
}

SpectrumSample dialectric_reflectance(float cos_theta_i, const SpectrumSample& ior, const SpectrumSample& absorption) {
    std::array<float, N_SPECTRUM_SAMPLES> ss{};
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; i++) {
        ss[i] = dielectric_reflectance(cos_theta_i, std::complex(ior[i], absorption[i]));
    }
    return SpectrumSample(std::move(ss));
}



SpectrumSample ConductorBxDF::operator()(Vec3 wo, Vec3 wi) const {
    // if (wo.z * wi.z <= 0.0f) {
    //     return SpectrumSample(0.0f);
    // }
    return SpectrumSample(0.0f);
    // todo: allow for rough surfaces
}

std::optional<BSDFSample> ConductorBxDF::sample(Vec3 wo, Sampler& sampler) const {
    Vec3 wi(-wo.x, -wo.y, wo.z);
    auto spec = dialectric_reflectance(abs_cos_theta(wi), m_ior, m_absorption) / abs_cos_theta(wi);
    return BSDFSample {
        .spec = spec,
        .wi = wi,
        .pdf = 1.0f
    };
    // todo: allow for rough surfaces
}

float ConductorBxDF::pdf(Vec3 wo, Vec3 wi, const Sampler& sampler) const {
    return 0.0f;
    // todo: allow for rough surfaces
}


SpectrumSample DielectricBxDF::operator()(Vec3 wo, Vec3 wi) const {
    return SpectrumSample(0.0f);
}

std::optional<BSDFSample> DielectricBxDF::sample(Vec3 wo, Sampler& sampler) const {
    float r = dielectric_reflectance(cos_theta(wo), m_ior);
    float t = 1.0f - r;
    if (sampler.sample_1d() < r / (r + t)) {
        // perfect reflection
        Vec3 wi(-wo.x, -wo.y, wo.z);
        SpectrumSample spec_r(r / abs_cos_theta(wi));
        return BSDFSample {
            .spec = spec_r,
            .wi = wi,
            .pdf = r / (r + t)
        };
    }
    else {
        // perfect transmission
        auto refr = refract(wo, Vec3(0.0f, 0.0f, 1.0f), m_ior);
        if (!refr) {
            return std::nullopt;
        }
        auto [wi, ior_p] = refr.value();
        SpectrumSample spec_r(t / abs_cos_theta(wi));
        // TODO: account for difference between radiance and importance
        return BSDFSample {
            .spec = spec_r,
            .wi = wi,
            .pdf = t / (r + t)
        };
    }
}

float DielectricBxDF::pdf(Vec3 wo, Vec3 wi, const Sampler& sampler) const {
    return 0.0f;
}


SpectrumSample ThinDielectricBxDF::operator()(Vec3 wo, Vec3 wi) const {
    return SpectrumSample(0.0f);
}

std::optional<BSDFSample> ThinDielectricBxDF::sample(Vec3 wo, Sampler& sampler) const {
    float r = dielectric_reflectance(cos_theta(wo), m_ior);
    float t = 1.0f - r;
    if (r < 1.0f) {
        r += t * t * r / (1.0f - r * r);
        t = 1.0f - r;
    }
    if (sampler.sample_1d() < r / (r + t)) {
        // perfect reflection
        Vec3 wi(-wo.x, -wo.y, wo.z);
        SpectrumSample spec_r(r / abs_cos_theta(wi));
        return BSDFSample {
            .spec = spec_r,
            .wi = wi,
            .pdf = r / (r + t)
        };
    }
    else {
        Vec3 wi = -wo;
        SpectrumSample spec_t(t / abs_cos_theta(wi));
        return BSDFSample {
            .spec = spec_t,
            .wi = wi,
            .pdf = t / (r + t)
        };
    }
}


float ThinDielectricBxDF::pdf(Vec3 wo, Vec3 wi, const Sampler& sampler) const {
    return 0.0f;
}