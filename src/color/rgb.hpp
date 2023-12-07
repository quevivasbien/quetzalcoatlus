#pragma once

#include <memory>

#include "spectrum.hpp"
#include "xyz.hpp"
#include "transform.hpp"
#include "vec.hpp"

class RGB : public Vec3 {
public:
    RGB() : Vec3(0.0f, 0.0f, 0.0f) {}
    explicit RGB(Vec3&& v) : Vec3(std::move(v)) {}
    RGB(float r, float g, float b) : Vec3(r, g, b) {}
};


// a polynomial that represents a spectrum of wavelengths
class RGBSigmoidPolynomial : public Spectrum {
public:
    RGBSigmoidPolynomial() {}
    RGBSigmoidPolynomial(float c0, float c1, float c2) : c0(c0), c1(c1), c2(c2) {}

    float operator()(float lambda) const override;

    float max_value() const;

    float c0, c1, c2;
};

class RGBToSpectrumTable {
public:
    static std::shared_ptr<const RGBToSpectrumTable> sRGB();

    RGBToSpectrumTable(std::vector<float>&& z_nodes, std::vector<float>&& coeffs) : m_z_nodes(std::move(z_nodes)), m_coeffs(std::move(coeffs)) {}

    // convert RGB to a polynomial representing a continuous spectrum
    RGBSigmoidPolynomial operator()(const RGB& rgb) const;

    std::vector<float> m_z_nodes;
    std::vector<float> m_coeffs;
};

class RGBColorSpace {
public:
    RGBColorSpace(
        Vec2 r, Vec2 g, Vec2 b,
        std::shared_ptr<const Spectrum> illuminant,
        std::shared_ptr<const RGBToSpectrumTable> table
    );

    RGB rgb_from_xyz(const XYZ& xyz) const;
    XYZ rgb_to_xyz(const RGB& rgb) const;

    RGB rgb_from_sample(const SpectrumSample& ss) const;

    RGBSigmoidPolynomial to_spectrum(const RGB& rgb) const;

    Vec2 whitepoint() const {
        return m_white;
    }

    static std::shared_ptr<const RGBColorSpace> sRGB();

    const Vec2 m_r;
    const Vec2 m_g;
    const Vec2 m_b;
    const std::shared_ptr<const Spectrum> m_illuminant;
    const std::shared_ptr<const RGBToSpectrumTable> m_table;

private:
    Vec2 m_white;
    Mat3 m_xyz_from_rgb;
    Mat3 m_rgb_from_xyz;
};


class RGBUnboundedSpectrum : public Spectrum {
public:
    RGBUnboundedSpectrum(const RGBSigmoidPolynomial& polynomial, float scale) : m_scale(scale), m_polynomial(polynomial) {};

    explicit RGBUnboundedSpectrum(RGB rgb, const RGBColorSpace& cs = *RGBColorSpace::sRGB());

    float operator()(float lambda) const override;

private:
    float m_scale;
    RGBSigmoidPolynomial m_polynomial;
};


class RGBIlluminantSpectrum : public Spectrum {
public:
    explicit RGBIlluminantSpectrum(RGB rgb, const RGBColorSpace& cs = *RGBColorSpace::sRGB());

    float operator()(float lambda) const override;

    const std::shared_ptr<const Spectrum> m_illuminant;

private:
    float m_scale;
    RGBSigmoidPolynomial m_polynomial;
};
