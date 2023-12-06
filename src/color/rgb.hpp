#pragma once

#include <memory>

#include "spectrum.hpp"
#include "xyz.hpp"
#include "transform.hpp"
#include "vec.hpp"

class RGB : public Vec3 {
public:
    explicit RGB(Vec3&& v) : Vec3(std::move(v)) {}
    RGB(float r, float g, float b) : Vec3(r, g, b) {}
};


// a polynomial that represents a spectrum of wavelengths
class RGBSigmoidPolynomial {
public:
    RGBSigmoidPolynomial(float c0, float c1, float c2) : c0(c0), c1(c1), c2(c2) {}

    float operator()(float lambda) const;

    float max_value() const;

    float c0, c1, c2;
};

const size_t SPECTRUM_TABLE_RES = 64;

class RGBToSpectrumTable {
public:
    using CoeffArray = float[3][SPECTRUM_TABLE_RES][SPECTRUM_TABLE_RES][SPECTRUM_TABLE_RES][3];

    static std::shared_ptr<const RGBToSpectrumTable> sRGB();

    RGBToSpectrumTable(const std::array<float, SPECTRUM_TABLE_RES>& z_nodes, const CoeffArray& coeffs) : m_z_nodes(z_nodes), m_coeffs(coeffs) {}

    // convert RGB to a polynomial representing a continuous spectrum
    RGBSigmoidPolynomial operator()(const RGB& rgb) const;

    const std::array<float, SPECTRUM_TABLE_RES>& m_z_nodes;
    const CoeffArray& m_coeffs;
};

class RGBColorSpace {
public:
    RGBColorSpace(
        Vec2 r, Vec2 g, Vec2 b,
        std::shared_ptr<const Spectrum> illuminant,
        std::shared_ptr<const RGBToSpectrumTable> table
    );

    RGB from_xyz(const XYZ& xyz) const;
    XYZ to_xyz(const RGB& rgb) const;

    RGB from_sample(const SpectrumSample& ss) const;

    RGBSigmoidPolynomial to_spectrum(const RGB& rgb) const;

    Vec2 whitepoint() const {
        return m_white;
    }

    std::tuple<Vec2, Vec2, Vec2> bounds() const {
        return {m_r, m_g, m_b};
    }

    static std::shared_ptr<const RGBColorSpace> sRGB();

private:
    Vec2 m_r;
    Vec2 m_g;
    Vec2 m_b;
    Vec2 m_white;
    Mat3 m_xyz_from_rgb;
    Mat3 m_rgb_from_xyz;
    std::shared_ptr<const Spectrum> m_illuminant;
    std::shared_ptr<const RGBToSpectrumTable> m_table;
};
