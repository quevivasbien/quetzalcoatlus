#include "interaction.hpp"
#include "light.hpp"

SpectrumSample PointLight::total_emission(const WavelengthSample& wavelengths) const {
    return SpectrumSample::from_spectrum(*m_spectrum, wavelengths) * (4.0f * M_PI * m_scale);
}

std::optional<LightSample> PointLight::sample(const SurfaceInteraction& si, const WavelengthSample& wavelengths, Sampler& sampler) const {
    Vec3 wi = (m_point - si.point).normalize();
    auto spec = SpectrumSample::from_spectrum(*m_spectrum, wavelengths) * (m_scale / (m_point - si.point).norm_squared());
    return LightSample {
        .spec = spec,
        .wi = wi,
        .pdf = 1.0f,
        .p_light = m_point
    };
}


SpectrumSample AreaLight::total_emission(const WavelengthSample& wavelengths) const {
    auto spec = SpectrumSample::from_spectrum(*m_spectrum, wavelengths);
    return spec * (M_PI * (m_two_sided ? 2.0f : 1.0f) * m_shape->area() * m_scale);
}


std::optional<LightSample> AreaLight::sample(const SurfaceInteraction& si, const WavelengthSample& wavelengths, Sampler& sampler) const {
    auto ss = m_shape->sample_point(sampler);
    if (ss.pdf == 0.0f || (ss.p - si.point).norm_squared() == 0.0f) {
        return std::nullopt;
    }
    Vec3 wi = (ss.p - si.point).normalize();
    auto spec = emission(ss.p, ss.normal, -wi, wavelengths);
    if (spec.is_zero()) {
        return std::nullopt;
    }
    return LightSample {
        .spec = spec,
        .wi = wi,
        .pdf = ss.pdf,
        .p_light = ss.p
    };
}

float AreaLight::pdf(const Pt3& p, const Vec3& wi) const {
    return m_shape->pdf(p);
}

SpectrumSample AreaLight::emission(const Pt3& p, const Vec3& n, const Vec3& w, const WavelengthSample& wavelengths) const {
    if (!m_two_sided && n.dot(w) < 0.0f) {
        return SpectrumSample(0.0f);
    }
    return SpectrumSample::from_spectrum(*m_spectrum, wavelengths) * m_scale;
}
