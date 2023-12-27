#pragma once

#include "rgb.hpp"
#include "spectra.hpp"
#include "spectrum.hpp"

// for converting sampled spectra to RGB pixel values
class PixelSensor {
public:
    PixelSensor(
        const RGBColorSpace& cs,
        const Spectrum& illuminant,
        float imaging_ratio = 1.0f
    );

    PixelSensor(
        const Spectrum& r,
        const Spectrum& g,
        const Spectrum& b,
        const RGBColorSpace& cs,
        const Spectrum& illuminant,
        float imaging_ratio = 1.0f
    );

    RGB to_sensor_rgb(const SpectrumSample& sample, const WavelengthSample& wavelengths) const;

    static PixelSensor CIE_XYZ(float imaging_ratio = 1.0f / spectra::CIE_Y_INTEGRAL);
    static PixelSensor CANON_EOS(float imaging_ratio = 1.0f / spectra::CANON_EOS_R()->integral());

private:
    DenselySampledSpectrum m_r;
    DenselySampledSpectrum m_g;
    DenselySampledSpectrum m_b;
    float m_imaging_ratio;
    Mat3 m_xyz_from_sensor_rgb;
};
