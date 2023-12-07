#pragma once

#include "rgb.hpp"
#include "spectrum.hpp"
#include "transform.hpp"

// for converting sampled spectra to RGB pixel values
class PixelSensor {
public:
    PixelSensor(
        const RGBColorSpace& cs,
        const Spectrum& illuminant,
        float imaging_ratio
    );

    RGB to_sensor_rgb(const SpectrumSample& sample) const;

    static const PixelSensor& CIE_XYZ();

private:
    DenselySampledSpectrum m_r;
    DenselySampledSpectrum m_g;
    DenselySampledSpectrum m_b;
    float m_imaging_ratio;
    Mat3 m_xyz_from_sensor_rgb;
};
