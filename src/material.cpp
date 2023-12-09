#include "interaction.hpp"
#include "material.hpp"

BSDF DiffuseMaterial::bsdf(const SurfaceInteraction& si, const WavelengthSample& wavelengths) const {
    auto r = m_texture->value(si.uv, si.point, wavelengths);
    return BSDF(
        si.normal,
        std::make_unique<DiffuseBxDF>(std::move(r))
    );
}