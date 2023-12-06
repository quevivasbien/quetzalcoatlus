#include <iostream>

#include "spectrum.hpp"
#include "rgb.hpp"
#include "xyz.hpp"

int main() {
    auto srgb = RGBColorSpace::sRGB();
    auto whitepoint = srgb->whitepoint();
    std::cout << "whitepoint: " <<whitepoint.x << " " << whitepoint.y << " " << std::endl;
    auto spectrum = srgb->to_spectrum(RGB(1.0, 0.0, 0.0));
    std::cout << spectrum.c0 << " " << spectrum.c1 << " " << spectrum.c2 << std::endl;
    return 0;
}