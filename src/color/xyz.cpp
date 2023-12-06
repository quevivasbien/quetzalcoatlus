#include <cstddef>
#include <memory>
#include <vector>

#include "xyz.hpp"

XYZ XYZ::from_spectrum(const Spectrum& spectrum) {
    return XYZ(
        spectra::X()->inner_product(spectrum) / spectra::CIE_Y_INTEGRAL,
        spectra::Y()->inner_product(spectrum) / spectra::CIE_Y_INTEGRAL,
        spectra::Z()->inner_product(spectrum) / spectra::CIE_Y_INTEGRAL
    );
}

XYZ XYZ::from_sample(const SpectrumSample& ss) {
    auto sx = SpectrumSample::from_spectrum(*spectra::X(), ss.m_wavelengths);
    auto sy = SpectrumSample::from_spectrum(*spectra::Y(), ss.m_wavelengths);
    auto sz = SpectrumSample::from_spectrum(*spectra::Z(), ss.m_wavelengths);
    auto pdf = ss.new_from_pdf();
    float x = ((sx * ss) / pdf).average() / spectra::CIE_Y_INTEGRAL;
    float y = ((sy * ss) / pdf).average() / spectra::CIE_Y_INTEGRAL;
    float z = ((sz * ss) / pdf).average() / spectra::CIE_Y_INTEGRAL;
    return XYZ(x, y, z);
}

Vec2 XYZ::xy() const {
    return Vec2(x / (x + y + z), y / (x + y + z));
}

XYZ XYZ::from_xyY(float x, float y, float Y) {
    if (y == 0.0f) {
        return XYZ(0.0f, 0.0f, 0.0f);
    }
    return XYZ(x * Y / y, Y, (1.0f - x - y) * Y / y);
}