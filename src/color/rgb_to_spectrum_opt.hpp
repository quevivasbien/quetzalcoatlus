#pragma once

#include <utility>
#include <vector>

namespace opt_rgb {

enum Gamut {
    SRGB,
    ProPhotoRGB,
    ACES2065_1,
    REC2020,
    ERGB,
    XYZ,
    DCI_P3,
    NO_GAMUT,
};

// get the coefficients for a given gamut at a given resolution
// will try to get the coefficients from the file {gamut}_{res}.dat
// if that doesn't exist, it will generate the coefficients (and save them after)
std::pair<std::vector<float>, std::vector<float>> get_coeffs(Gamut gamut, size_t res);

}