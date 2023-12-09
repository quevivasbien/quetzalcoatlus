#include "interaction.hpp"
#include "light.hpp"

SpectrumSample SimpleLight::emission(const SurfaceInteraction& si, const Vec3& wo, const WavelengthSample& wavelengths) const {
    return SpectrumSample::from_spectrum(*m_radiance, wavelengths);
}

SpectrumSample DirectionalLight::emission(const SurfaceInteraction& si, const Vec3& wo, const WavelengthSample& wavelengths) const {
    return SpectrumSample::from_spectrum(*m_radiance, wavelengths) * si.normal.dot(wo);
}