#pragma once

#include "spectrum.hpp"
#include "spectrum_sample.hpp"
#include "vec.hpp"


class XYZ : public Vec3 {
public:
    explicit XYZ(Vec3&& v) : Vec3(std::move(v)) {}
    XYZ(float x, float y, float z) : Vec3(x, y, z) {}

    static XYZ from_spectrum(const Spectrum& spectrum);
    
    static XYZ from_sample(const SpectrumSample& ss, const WavelengthSample& wavelengths);

    Vec2 xy() const;

    static XYZ from_xyY(float x, float y, float Y = 1.0f);
};

