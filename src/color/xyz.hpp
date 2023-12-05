#pragma once

#include "spectrum.hpp"
#include "spectrum_sample.hpp"
#include "../vec.hpp"

namespace spectra {
    const size_t N_CIE_SAMPLES = 471;
    const float CIE_Y_INTEGRAL = 106.856895f;

    const DenselySampledSpectrum& X();
    const DenselySampledSpectrum& Y();
    const DenselySampledSpectrum& Z();
}

class XYZ : public Vec3 {
public:
    explicit XYZ(Vec3&& v) : Vec3(std::move(v)) {}
    XYZ(float x, float y, float z) : Vec3(x, y, z) {}

    static XYZ from_spectrum(const Spectrum& spectrum);
    
    template <size_t N>
    static XYZ from_sample(const SpectrumSample<N>& ss) {
        auto x = spectra::X().sample(ss.m_wavelengths);
        auto y = spectra::Y().sample(ss.m_wavelengths);
        auto z = spectra::Z().sample(ss.m_wavelengths);
        auto pdf = ss.new_from_pdf();
        x = ((x * ss) / pdf).average() / spectra::CIE_Y_INTEGRAL;
        y = ((y * ss) / pdf).average() / spectra::CIE_Y_INTEGRAL;
        z = ((z * ss) / pdf).average() / spectra::CIE_Y_INTEGRAL;
        return XYZ(x, y, z);
    }

    Vec2 xy() const;

    static XYZ from_xyY(float x, float y, float Y = 1.0f);
};

