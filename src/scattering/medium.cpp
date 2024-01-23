#include "medium.hpp"

std::optional<RayMajorantSegment> HomogeneousMajorantIterator::next() {
    if (m_segment) {
        auto seg = m_segment.value();
        m_segment = std::nullopt;
        return seg;
    }
    return std::nullopt;
}

MediumSample HomogeneousMedium::sample_point(Pt3 p, const WavelengthSample& wavelengths) const {
    return {
        .absorption = SpectrumSample::from_spectrum(*m_absorption, wavelengths),
        .scattering = SpectrumSample::from_spectrum(*m_scattering, wavelengths),
        .phase = phase
    };
}

std::unique_ptr<RayMajorantIterator> HomogeneousMedium::sample_ray(Ray ray, float t_max, const WavelengthSample& wavelengths) const {
    auto absorption = SpectrumSample::from_spectrum(*m_absorption, wavelengths);
    auto scattering = SpectrumSample::from_spectrum(*m_scattering, wavelengths);
    return std::make_unique<HomogeneousMajorantIterator>(
        absorption + scattering,
        0.0f, t_max
    );
}