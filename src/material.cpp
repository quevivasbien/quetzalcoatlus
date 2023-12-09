#include "interaction.hpp"
#include "material.hpp"

BSDF DiffuseMaterial::bsdf(const SurfaceInteraction& si, WavelengthSample& wavelengths) const {
    auto r = m_texture->value(si.uv, si.point, wavelengths);
    return BSDF(
        si.normal,
        std::make_unique<DiffuseBxDF>(std::move(r))
    );
}

BSDF ConductiveMaterial::bsdf(const SurfaceInteraction& si, WavelengthSample& wavelengths) const {
    auto ior = SpectrumSample::from_spectrum(*m_ior, wavelengths);
    auto absorption = SpectrumSample::from_spectrum(*m_absorption, wavelengths);

    return BSDF(
        si.normal,
        std::make_unique<ConductorBxDF>(std::move(ior), std::move(absorption))
    );
}

BSDF DielectricMaterial::bsdf(const SurfaceInteraction& si, WavelengthSample& wavelengths) const {
    float ior = (*m_ior)(wavelengths[0]);
    if (!is_constant) {
        wavelengths.terminate_secondary();
    }

    return BSDF(
        si.normal,
        std::make_unique<DielectricBxDF>(ior)
    );
}