#pragma once

#include <memory>

#include "spectrum.hpp"
#include "xyz.hpp"
#include "../transform.hpp"
#include "../vec.hpp"

class RGB : public Vec3 {
public:
    explicit RGB(Vec3&& v) : Vec3(std::move(v)) {}
    RGB(float r, float g, float b) : Vec3(r, g, b) {}
};

class RGBToSpectrumTable {
  // TODO  
};

class RGBColorSpace {
public:
    RGBColorSpace(
        Vec2 r, Vec2 g, Vec2 b,
        const std::shared_ptr<Spectrum>& illuminant,
        const std::shared_ptr<RGBToSpectrumTable>& table
    );

    RGB from_xyz(const XYZ& xyz) const;
    XYZ to_xyz(const RGB& rgb) const;

    RGB from_sample(const SpectrumSample<3>& ss) const;

    Vec2 m_b;
    Vec2 m_g;
    Vec2 m_r;
    Vec2 m_white;
    Mat3 m_xyz_from_rgb;
    Mat3 m_rgb_from_xyz;
    std::shared_ptr<Spectrum> m_illuminant;
    std::shared_ptr<RGBToSpectrumTable> m_table;
};

namespace colorspace {
    const RGBColorSpace sRGB();
}