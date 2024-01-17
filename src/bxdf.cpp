#include <algorithm>
#include <cmath>
#include <complex>

#include "bxdf.hpp"
#include "util.hpp"

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

float cos_phi(Vec3 w) {
    float sin_theta_ = sin_theta(w);
    return (sin_theta_ == 0.0f) ? 1.0f : std::clamp(w.x / sin_theta_, -1.0f, 1.0f);
}

float sin_phi(Vec3 w) {
    float sin_theta_ = sin_theta(w);
    return (sin_theta_ == 0.0f) ? 0.0f : std::clamp(w.y / sin_theta_, -1.0f, 1.0f);
}


SpectrumSample BxDF::rho_hd(Vec3 wo, Sampler& sampler, size_t n_samples) const {
    SpectrumSample r(0.0f);
    for (size_t i = 0; i < n_samples; i++) {
        auto bs = sample(wo, sampler.sample_1d(), sampler.sample_2d());
        if (bs) {
            r += bs->spec * abs_cos_theta(bs->wi) / bs->pdf;
        }
    }
    return r / float(n_samples);
}

SpectrumSample BxDF::rho_hh(Sampler& sampler, size_t n_samples) const {
    SpectrumSample r(0.0f);
    for (size_t i = 0; i < n_samples; i++) {
        Vec3 wo = sampler.sample_uniform_hemisphere();
        if (wo.z == 0.0f) {
            continue;
        }
        float pdf_o = Sampler::uniform_hemisphere_pdf();
        auto bs = sample(wo, sampler.sample_1d(), sampler.sample_2d());
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

std::optional<BSDFSample> BSDF::sample(Vec3 wo_render, float sample1, Vec2 sample2) const {
    Vec3 wo = local_from_render(wo_render);
    if (wo.z == 0.0f) {
        return std::nullopt;
    }
    auto bs = (*m_bxdf).sample(wo, sample1, sample2);
    if (!bs || bs->spec.is_zero() || bs->pdf == 0.0f || bs->wi.z == 0.0f) {
        return std::nullopt;
    }
    bs->wi = render_from_local(bs->wi);
    return *bs;
}

float BSDF::pdf(Vec3 wo_render, Vec3 wi_render) const {
    Vec3 wi = local_from_render(wi_render);
    Vec3 wo = local_from_render(wo_render);
    if (wo.z == 0.0f) {
        return 0.0f;
    }
    return m_bxdf->pdf(wo, wi);
}

SpectrumSample BSDF::rho_hd(Vec3 wo_render, Sampler& sampler, size_t n_samples) const {
    return m_bxdf->rho_hd(local_from_render(wo_render), sampler, n_samples);
}

SpectrumSample BSDF::rho_hh(Sampler& sampler, size_t n_samples) const {
    return m_bxdf->rho_hh(sampler, n_samples);
}


SpectrumSample DiffuseBxDF::operator()(Vec3 wo, Vec3 wi) const {
    if (wo.z * wi.z <= 0.0f) {
        return SpectrumSample(0.0f);
    }
    return m_reflectance * M_1_PI;
}

std::optional<BSDFSample> DiffuseBxDF::sample(Vec3 wo, float sample1, Vec2 sample2) const {
    Vec3 wi = Sampler::sample_cosine_hemisphere(sample2);
    if (wo.z < 0.0f) {
        wi.z *= -1.0f;
    }
    float pdf_ = Sampler::cosine_hemisphere_pdf(abs_cos_theta(wi));
    return BSDFSample {
        .spec = m_reflectance * M_1_PI,
        .wi = wi,
        .pdf = pdf_
    };
}

float DiffuseBxDF::pdf(Vec3 wo, Vec3 wi) const {
    if (wo.z * wi.z <= 0.0f) {
        return 0.0f;
    }
    return Sampler::cosine_hemisphere_pdf(abs_cos_theta(wi));
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

SpectrumSample dielectric_reflectance(float cos_theta_i, const SpectrumSample& ior, const SpectrumSample& absorption) {
    std::array<float, N_SPECTRUM_SAMPLES> ss{};
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; i++) {
        ss[i] = dielectric_reflectance(cos_theta_i, std::complex(ior[i], absorption[i]));
    }
    return SpectrumSample(std::move(ss));
}

TrowbridgeReitzDistribution::TrowbridgeReitzDistribution(float alpha_x, float alpha_y) : m_alpha_x(alpha_x), m_alpha_y(alpha_y) {
    if (!is_smooth() && (alpha_x == 0. || alpha_y == 0.)) {
        m_alpha_x = std::max(1e-5f, alpha_x);
        m_alpha_y = std::max(1e-5f, alpha_y);
    }
}

float TrowbridgeReitzDistribution::operator()(Vec3 wm) const {
    float tan2_theta_ = tan2_theta(wm);
    if (std::isinf(tan2_theta_)) {
        return 0.0f;
    }
    float cos4_theta = std::pow(cos2_theta(wm), 2);
    float e = tan2_theta_ * (
        std::pow(cos_phi(wm) / m_alpha_x, 2)
        + std::pow(sin_phi(wm) / m_alpha_y, 2)
    );
    return 1.0f / (M_PI * m_alpha_x * m_alpha_y * cos4_theta * (1.0f + e) * (1.0f + e));
}

float TrowbridgeReitzDistribution::operator()(Vec3 w, Vec3 wm) const {
    return (masking(w) / std::abs(cos_theta(w))) * (*this)(wm) * std::abs(w.dot(wm));
}

bool TrowbridgeReitzDistribution::is_smooth() const {
    return m_alpha_x < 1e-3f && m_alpha_y < 1e-3f; 
}

float TrowbridgeReitzDistribution::lambda(Vec3 w) const {
    float tan2_theta_ = tan2_theta(w);
    if (std::isinf(tan2_theta_)) {
        return 0.0f;
    }
    float alpha2 = std::pow(cos_phi(w) * m_alpha_x, 2) + std::pow(sin_phi(w) * m_alpha_y, 2);
    return (std::sqrt(1.0f + alpha2 * tan2_theta_) - 1.0f) / 2.0f;
}

float TrowbridgeReitzDistribution::masking(Vec3 w) const {
    return 1.0f / (1.0f + lambda(w));
}

float TrowbridgeReitzDistribution::masking_shadowing(Vec3 wo, Vec3 wi) const {
    return 1.0f / (1.0f + lambda(wo) + lambda(wi));
}

Vec3 TrowbridgeReitzDistribution::sample(Vec3 w, Vec2 sample2) const {
    Vec3 wh = Vec3(m_alpha_x * w.x, m_alpha_y * w.y, w.z).normalized();
    if (wh.z < 0.0f) {
        wh = -wh;
    }
    Vec3 t1 = wh.z < 0.99999f ? Vec3(0.0f, 0.0f, 1.0f).cross(wh).normalized() : Vec3(1.0f, 0.0f, 0.0f);
    Vec3 t2 = wh.cross(t1);
    Vec2 p = Sampler::sample_uniform_disk_polar(sample2);
    float h = std::sqrt(1.0f - p.x * p.x);
    p.y = lerp(h, p.y, (1.0 + wh.z) * 0.5f);
    float pz = std::sqrt(std::max(0.0f, 1.0f - p.norm_squared()));
    Vec3 nh = p.x * t1 + p.y * t2 + pz * wh;
    return Vec3(
        m_alpha_x * nh.x,
        m_alpha_y * nh.y,
        std::max(1e-6f, nh.z)
    ).normalized();
}


SpectrumSample ConductorBxDF::operator()(Vec3 wo, Vec3 wi) const {
    if (wo.z * wi.z <= 0.0f || m_roughness.is_smooth()) {
        return SpectrumSample(0.0f);
    }
    float cos_theta_o = std::abs(cos_theta(wo));
    float cos_theta_i = std::abs(cos_theta(wi));
    if (cos_theta_i == 0.0f || cos_theta_o == 0.0f) {
        return SpectrumSample(0.0f);
    }
    Vec3 wm = wi + wo;
    if (wm.norm_squared() == 0.0f) {
        return SpectrumSample(0.0f);
    }
    wm = wm.normalized();
    auto fresnel_factor = dielectric_reflectance(std::abs(wo.dot(wm)), m_ior, m_absorption);
    return fresnel_factor * (m_roughness(wm) * m_roughness.masking_shadowing(wo, wi) / (4.0f * cos_theta_i * cos_theta_o));
}

// todo: figure out why steep oncoming angles don't work
std::optional<BSDFSample> ConductorBxDF::sample(Vec3 wo, float sample1, Vec2 sample2) const {
    if (m_roughness.is_smooth()) {
        Vec3 wi(-wo.x, -wo.y, wo.z);
        auto spec = dielectric_reflectance(abs_cos_theta(wi), m_ior, m_absorption) / abs_cos_theta(wi);
        return BSDFSample {
            .spec = spec,
            .wi = wi,
            .pdf = 1.0f,
            .scatter_type = ScatterType {
                .specular = true
            },
        };
    }
    Vec3 wm = m_roughness.sample(wo, sample2);
    Vec3 wi = reflect(wo, wm);
    if (wo.z * wi.z <= 0.0f) {
        return std::nullopt;
    }
    float pdf_ = m_roughness(wo, wm)  / (4.0f * std::abs(wo.dot(wm)));
    float cos_theta_o = std::abs(cos_theta(wo));
    float cos_theta_i = std::abs(cos_theta(wi));
    auto fresnel_factor = dielectric_reflectance(std::abs(wo.dot(wm)), m_ior, m_absorption);
    auto spec = fresnel_factor * (m_roughness(wm) * m_roughness.masking_shadowing(wo, wi) / (4.0f * cos_theta_i * cos_theta_o));
    return BSDFSample {
        .spec = spec,
        .wi = wi,
        .pdf = pdf_
    };
}

float ConductorBxDF::pdf(Vec3 wo, Vec3 wi) const {
    if (wo.z * wi.z <= 0.0f || m_roughness.is_smooth()) {
        return 0.0f;
    }
    auto wm = wo + wi;
    if (wm.norm_squared() == 0.0f) {
        return 0.0f;
    }
    wm = wm.dot(Vec3(0.0f, 0.0f, 1.0f)) > 0.0f ? wm.normalized() : -wm.normalized();
    return m_roughness(wo, wm) / (4.0f * std::abs(wo.dot(wm))); 
}

bool ConductorBxDF::is_specular() const {
    return m_roughness.is_smooth();
}


SpectrumSample DielectricBxDF::operator()(Vec3 wo, Vec3 wi) const {
    return SpectrumSample(0.0f);
}

std::optional<BSDFSample> DielectricBxDF::sample(Vec3 wo, float sample1, Vec2 sample2) const {
    float r = dielectric_reflectance(cos_theta(wo), m_ior);
    float t = 1.0f - r;
    if (sample1 < r / (r + t)) {
        // perfect reflection
        Vec3 wi(-wo.x, -wo.y, wo.z);
        SpectrumSample spec_r(r / abs_cos_theta(wi));
        return BSDFSample {
            .spec = spec_r,
            .wi = wi,
            .pdf = r / (r + t),
            .scatter_type = ScatterType {
                .specular = true
            }
        };
    }
    else {
        // perfect transmission
        auto refr = refract(wo, Vec3(0.0f, 0.0f, 1.0f), m_ior);
        if (!refr) {
            return std::nullopt;
        }
        auto [wi, ior_p] = refr.value();
        SpectrumSample spec_t(t / abs_cos_theta(wi));
        // TODO: account for difference between radiance and importance
        return BSDFSample {
            .spec = spec_t,
            .wi = wi,
            .pdf = t / (r + t),
            .ior = ior_p,
            .scatter_type = ScatterType {
                .specular = true,
                .transmission = true
            },
        };
    }
}

float DielectricBxDF::pdf(Vec3 wo, Vec3 wi) const {
    return 0.0f;
}


SpectrumSample ThinDielectricBxDF::operator()(Vec3 wo, Vec3 wi) const {
    return SpectrumSample(0.0f);
}

std::optional<BSDFSample> ThinDielectricBxDF::sample(Vec3 wo, float sample1, Vec2 sample2) const {
    float r = dielectric_reflectance(cos_theta(wo), m_ior);
    float t = 1.0f - r;
    if (r < 1.0f) {
        r += t * t * r / (1.0f - r * r);
        t = 1.0f - r;
    }
    if (sample1 < r / (r + t)) {
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


float ThinDielectricBxDF::pdf(Vec3 wo, Vec3 wi) const {
    return 0.0f;
}