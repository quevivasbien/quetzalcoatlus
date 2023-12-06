#include <iostream>

#include "spectrum.hpp"
#include "rgb.hpp"
#include "xyz.hpp"

int main() {
    auto srgb = RGBColorSpace::sRGB();
    auto whitepoint = srgb->whitepoint();
    // should be about [0.3127, 0.3290]
    std::cout << "whitepoint: " << whitepoint.x << " " << whitepoint.y << " " << std::endl;

    auto white = XYZ::from_spectrum(srgb->to_spectrum(RGB(1, 1, 1))).xy();
    std::cout << "Should be approximately equal to whitepoint:" << std::endl;
    std::cout << white.x << " " << white.y << std::endl;

    auto spectrum = srgb->to_spectrum(RGB(0.7, 0.5, 0.8));
    // std::cout << spectrum.c0 << " " << spectrum.c1 << " " << spectrum.c2 << std::endl;
    auto xyz = XYZ::from_spectrum(spectrum);
    auto rgb = srgb->rgb_from_xyz(xyz);
    std::cout << "Should be close to [0.7, 0.5, 0.8]:" << std::endl;
    std::cout << rgb.x << " " << rgb.y << " " << rgb.z << std::endl;

    return 0;
}