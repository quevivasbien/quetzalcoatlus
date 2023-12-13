#include "interaction.hpp"
#include "material.hpp"

BSDF DiffuseMaterial::bsdf(const SurfaceInteraction& si, WavelengthSample& wavelengths, Sampler& sampler) const {
    auto r = m_texture->value(si.uv, si.point, wavelengths);
    return BSDF(
        si.normal,
        std::make_unique<DiffuseBxDF>(std::move(r))
    );
}

BSDF ConductiveMaterial::bsdf(const SurfaceInteraction& si, WavelengthSample& wavelengths, Sampler& sampler) const {
    auto ior = SpectrumSample::from_spectrum(*m_ior, wavelengths);
    auto absorption = SpectrumSample::from_spectrum(*m_absorption, wavelengths);

    return BSDF(
        si.normal,
        std::make_unique<ConductorBxDF>(std::move(ior), std::move(absorption))
    );
}

ConductiveMaterial ConductiveMaterial::alluminum() {
    return ConductiveMaterial(
        spectra::AL_IOR(),
        spectra::AL_ABSORPTION()
    );
}

ConductiveMaterial ConductiveMaterial::copper() {
    return ConductiveMaterial(
        spectra::CU_IOR(),
        spectra::CU_ABSORPTION()
    );
}

BSDF DielectricMaterial::bsdf(const SurfaceInteraction& si, WavelengthSample& wavelengths, Sampler& sampler) const {
    float ior = (*m_ior)(wavelengths[0]);
    if (!is_constant) {
        wavelengths.terminate_secondary();
    }

    return BSDF(
        si.normal,
        std::make_unique<DielectricBxDF>(ior)
    );
}

BSDF ThinDielectricMaterial::bsdf(const SurfaceInteraction& si, WavelengthSample& wavelengths, Sampler& sampler) const {
    float ior = (*m_ior)(wavelengths[0]);
    if (!is_constant) {
        wavelengths.terminate_secondary();
    }

    return BSDF(
        si.normal,
        std::make_unique<ThinDielectricBxDF>(ior)
    );
}
