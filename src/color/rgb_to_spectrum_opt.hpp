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

std::pair<std::vector<float>, std::vector<float>> optimize_coeffs(Gamut gamut, size_t res);

}