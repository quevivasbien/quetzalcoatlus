#include "texture.hpp"

SolidColor::SolidColor(const RGB& color, const RGBColorSpace& cs) : spectrum(std::make_unique<RGBSigmoidPolynomial>(cs.to_spectrum(color))) {}

SpectrumSample SolidColor::value(
    const Vec2& uv,
    const Pt3& point,
    const WavelengthSample& lambdas
) const {
    return SpectrumSample::from_wavelengths(lambdas);
}


DummyTexture::DummyTexture() :
    white(RGBColorSpace::sRGB()->to_spectrum(RGB(1., 1., 1.))),
    black(RGBColorSpace::sRGB()->to_spectrum(RGB(0., 0., 0.)))
{}

SpectrumSample DummyTexture::value(
    const Vec2& uv,
    const Pt3& point,
    const WavelengthSample& lambdas
) const {
    float u = uv.x * 10.;
    float v = uv.y * 10.;
    if (int(floorf(u) + floorf(v)) % 2 == 0) {
        return SpectrumSample::from_spectrum(
            white, lambdas
        );
    }
    else {
        return SpectrumSample::from_spectrum(
            black, lambdas
        );
    }
}


SpectrumSample ImageTexture::value(
    const Vec2& uv,
    const Pt3& point,
    const WavelengthSample& lambdas
) const {
    size_t x = uv.x * image.width;
    if (x == image.width) {
        x = image.width - 1;
    }
    size_t y = uv.y * image.height;
    if (y == image.height) {
        y = image.height - 1;
    }
    RGB rgb(image.color_buffer[3 * (y * image.width + x) + 0],
            image.color_buffer[3 * (y * image.width + x) + 1],
            image.color_buffer[3 * (y * image.width + x) + 2]);

    auto spec = RGBColorSpace::sRGB()->to_spectrum(rgb);
    return SpectrumSample::from_spectrum(
        spec,
        lambdas
    );
}
