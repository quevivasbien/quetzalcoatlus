#pragma once

#include <cmath>
#include <memory>

#include "color/color.hpp"
#include "image.hpp"
#include "vec.hpp"

class Texture {
public:
    virtual SpectrumSample value(
        const Vec2& uv,
        const Pt3& point,
        const WavelengthSample& lambdas
    ) const = 0;
};


class SolidColor : public Texture {
public:
    SolidColor(const RGB& color, const RGBColorSpace& cs);

    SpectrumSample value(
        const Vec2& uv,
        const Pt3& point,
        const WavelengthSample& lambdas
    ) const override;

    std::unique_ptr<Spectrum> spectrum;
};


// texture used in testing to display uv coordinates
class DummyTexture : public Texture {
public:
    DummyTexture();

    SpectrumSample value(
        const Vec2& uv,
        const Pt3& point,
        const WavelengthSample& lambdas
    ) const override;

private:
    RGBSigmoidPolynomial white;
    RGBSigmoidPolynomial black;
};


class ImageTexture : public Texture {
public:
    explicit ImageTexture(Image&& image) : image(std::move(image)) {};

    SpectrumSample value(
        const Vec2& uv,
        const Pt3& point,
        const WavelengthSample& lambdas
    ) const override;

    Image image;
};