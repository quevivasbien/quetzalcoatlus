#include <iostream>

#include "color.hpp"

int main() {
    // check colorspace setup
    auto srgb = RGBColorSpace::sRGB();
    auto whitepoint = srgb->whitepoint();
    // should be about [0.3127, 0.3290]
    std::cout << "whitepoint: " << whitepoint.x << " " << whitepoint.y << " " << std::endl;

    // check spectrum to XYZ conversion
    auto white = XYZ::from_spectrum(srgb->to_spectrum(RGB(1, 1, 1))).xy();
    std::cout << "Should be approximately equal to whitepoint:" << std::endl;
    std::cout << white.x << " " << white.y << std::endl;

    // check round trip color conversion
    auto spectrum = srgb->to_spectrum(RGB(0.7, 0.5, 0.8));
    // std::cout << spectrum.c0 << " " << spectrum.c1 << " " << spectrum.c2 << std::endl;
    auto xyz = XYZ::from_spectrum(spectrum);
    auto rgb = srgb->rgb_from_xyz(xyz);
    std::cout << "Should be close to [0.7, 0.5, 0.8]:" << std::endl;
    std::cout << rgb.x << " " << rgb.y << " " << rgb.z << std::endl;

    // check PixelSensor SampledSpectrum -> RGB conversion

    std::cout << "Sensor sampled RGB values:" << std::endl;
    for (float x = 0.0f; x < 1.0f; x += 0.1f) {
        auto wl = WavelengthSample::uniform(x);
        auto sample = SpectrumSample::from_spectrum(spectrum, wl);
        auto color_out = PixelSensor::CIE_XYZ().to_sensor_rgb(sample, wl);
        std::cout << color_out.x << " " << color_out.y << " " << color_out.z << std::endl;
    }

    return 0;
}