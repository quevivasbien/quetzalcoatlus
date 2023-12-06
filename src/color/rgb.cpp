#include <algorithm>
#include <cmath>
#include <iostream>

#include "rgb_to_spectrum_opt.hpp"
#include "rgb.hpp"

const size_t SPECTRUM_TABLE_RES = 64;

RGBColorSpace::RGBColorSpace(
    Vec2 r, Vec2 g, Vec2 b,
    std::shared_ptr<const Spectrum> illuminant,
    std::shared_ptr<const RGBToSpectrumTable> table
) : m_r(r), m_g(g), m_b(b), m_illuminant(illuminant), m_table(table) {
    XYZ white = XYZ::from_spectrum(*m_illuminant);
    m_white = white.xy();
    XYZ R = XYZ::from_xyY(r.x, r.y);
    XYZ G = XYZ::from_xyY(g.x, g.y);
    XYZ B = XYZ::from_xyY(b.x, b.y);
    Mat3 rgb({
        R.x, G.x, B.x,
        R.y, G.y, B.y,
        R.z, G.z, B.z
    });
    XYZ C = XYZ(rgb.invert().value() * white);
    m_xyz_from_rgb = rgb * Mat3::diagonal({
        C.x, C.y, C.z
    });
    m_rgb_from_xyz = m_xyz_from_rgb.invert().value();
}

RGB RGBColorSpace::rgb_from_xyz(const XYZ& xyz) const {
    return RGB(m_rgb_from_xyz * xyz);
}
XYZ RGBColorSpace::rgb_to_xyz(const RGB& rgb) const {
    return XYZ(m_xyz_from_rgb * rgb);
}

RGB RGBColorSpace::rgb_from_sample(const SpectrumSample& ss) const {
    return rgb_from_xyz(XYZ::from_sample(ss));
}

RGBSigmoidPolynomial RGBColorSpace::to_spectrum(const RGB& rgb) const {
    return m_table->operator()(RGB(
        std::max(rgb.x, 0.0f),
        std::max(rgb.y, 0.0f),
        std::max(rgb.z, 0.0f)
    ));
}


float sigmoid(float x) {
    if (std::isinf(x)) {
        return x > 0.0f ? 1.0f : 0.0f;
    }
    return 0.5f + x / (2.0f * std::sqrt(1.0f + x * x));
}

float RGBSigmoidPolynomial::operator()(float lambda) const {
    return sigmoid(c0 + c1 * lambda + c2 * lambda * lambda);
}

float RGBSigmoidPolynomial::max_value() const {
    float result = std::max((*this)(LAMBDA_MIN), (*this)(LAMBDA_MAX));
    float lambda = -c1 / (2.0f * c0);
    if (lambda >= LAMBDA_MIN && lambda <= LAMBDA_MAX) {
        result = std::max(result, (*this)(lambda));
    }
    return result;
}

float lerp(float a, float b, float t) {
    return a * (1.0f - t) + b * t;
}

RGBSigmoidPolynomial RGBToSpectrumTable::operator()(const RGB& rgb) const {
    if (rgb.r() == rgb.g() && rgb.g() == rgb.b()) {
        // returns a constant spectrum
        return RGBSigmoidPolynomial(
            0.0f, 0.0f, (rgb.r() - 0.5f) / std::sqrt(rgb.r() * (1.0f - rgb.r()))
        );
    }

    float x, y, z;
    size_t maxc;
    // set z as largest component, and the others as ratios over z
    if (rgb.r() > rgb.g()) {
        if (rgb.r() > rgb.b()) {
            z = rgb.r();
            x = rgb.g() / z;
            y = rgb.b() / z;
            maxc = 0;
        } else {
            z = rgb.b();
            x = rgb.g() / z;
            y = rgb.r() / z;
            maxc = 2;
        }
    }
    else {
        if (rgb.g() > rgb.b()) {
            z = rgb.g();
            x = rgb.r() * (SPECTRUM_TABLE_RES - 1) / z;
            y = rgb.b() * (SPECTRUM_TABLE_RES - 1) / z;
            maxc = 1;
        } else {
            z = rgb.b();
            x = rgb.r() * (SPECTRUM_TABLE_RES - 1) / z;
            y = rgb.g() * (SPECTRUM_TABLE_RES - 1) / z;
            maxc = 2;
        }
    }
    size_t xi = std::min(size_t(x), SPECTRUM_TABLE_RES - 2);
    size_t yi = std::min(size_t(y), SPECTRUM_TABLE_RES - 2);
    const auto pp =  std::lower_bound(
        m_z_nodes.begin(),
        m_z_nodes.end(),
        z
    );
    size_t zi = std::distance(m_z_nodes.begin(), pp);
    if (zi != 0) {
        zi--;
    }
    if (zi > SPECTRUM_TABLE_RES - 2) {
        std::cout << "zi out of range, clamping to max" << std::endl;
        zi = SPECTRUM_TABLE_RES - 2;
    }
    std::array<float, 3> coeffs;
    float dx = x - xi;
    float dy = y - yi;
    float dz = (z - m_z_nodes[zi]) / (m_z_nodes[zi + 1] - m_z_nodes[zi]);
    for (size_t i = 0; i < 3; i++) {
        auto co = [&](size_t dxi, size_t dyi, size_t dzi) {
            // todo: represent the array more efficiently
            size_t index = maxc * SPECTRUM_TABLE_RES * SPECTRUM_TABLE_RES * SPECTRUM_TABLE_RES * 3
                + (zi + dzi) * SPECTRUM_TABLE_RES * SPECTRUM_TABLE_RES * 3
                + (yi + dyi) * SPECTRUM_TABLE_RES * 3
                + (xi + dxi) * 3
                + i;
            return m_coeffs[index];
        };
        coeffs[i] = lerp(
            lerp(
                lerp(co(0, 0, 0), co(1, 0, 0), dx),
                lerp(co(0, 1, 0), co(1, 1, 0), dx),
                dy
            ),
            lerp(
                lerp(co(0, 0, 1), co(1, 0, 1), dx),
                lerp(co(0, 1, 1), co(1, 1, 1), dx),
                dy
            ),
            dz
        );
    }

    return RGBSigmoidPolynomial(coeffs[2], coeffs[1], coeffs[0]);
}

std::shared_ptr<const RGBToSpectrumTable> RGBToSpectrumTable::sRGB() {
    static std::shared_ptr<const RGBToSpectrumTable> table;
    if (!table) {
        auto [scale, coeffs] = opt_rgb::get_coeffs(opt_rgb::Gamut::SRGB, SPECTRUM_TABLE_RES);
        table = std::make_shared<RGBToSpectrumTable>(
            std::move(scale), std::move(coeffs)
        );
    }
    return table;
}

std::shared_ptr<const RGBColorSpace> RGBColorSpace::sRGB() {
    static std::shared_ptr<const RGBColorSpace> space;
    if (!space) {
        space = std::make_shared<RGBColorSpace>(
            Vec2(0.64, 0.33),
            Vec2(0.3, 0.6),
            Vec2(0.15, 0.06),
            spectra::STD_ILLUM_D65(),
            RGBToSpectrumTable::sRGB()
        );
    }
    return space;
}