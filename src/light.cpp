#include "interaction.hpp"
#include "light.hpp"

SpectrumSample PointLight::total_emission(const WavelengthSample& wavelengths) const {
    return SpectrumSample::from_spectrum(*m_spectrum, wavelengths) * (4.0f * M_PI * m_scale);
}

SpectrumSample PointLight::emission(const SurfaceInteraction& si, const Vec3& wo, const WavelengthSample& wavelengths) const {
    return SpectrumSample(0.0f);
}

float PointLight::pdf(const SurfaceInteraction& si, const Vec3& wi) const {
    return 0.0f;
}

std::optional<LightSample> PointLight::sample(const SurfaceInteraction& si, const WavelengthSample& wavelengths, Sampler& sampler) const {
    Pt3 p = m_render_from_light * Pt3(0.0f, 0.0f, 0.0f);
    Vec3 wi = (p - si.point).normalize();
    auto spec = SpectrumSample::from_spectrum(*m_spectrum, wavelengths) * (m_scale / (p - si.point).norm_squared());
    return LightSample {
        .spec = spec,
        .wi = wi,
        .pdf = 1.0f,
        .p_light = p
    };
}