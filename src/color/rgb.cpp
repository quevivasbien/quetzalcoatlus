#include <algorithm>
#include <cmath>
#include <iostream>

#include "sRGB_spectrum_table.hpp"
#include "rgb.hpp"

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

RGB RGBColorSpace::from_xyz(const XYZ& xyz) const {
    return RGB(m_rgb_from_xyz * xyz);
}
XYZ RGBColorSpace::to_xyz(const RGB& rgb) const {
    return XYZ(m_xyz_from_rgb * rgb);
}

RGB RGBColorSpace::from_sample(const SpectrumSample& ss) const {
    return from_xyz(XYZ::from_sample(ss));
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
            x = rgb.r() / z;
            y = rgb.b() / z;
            maxc = 1;
        } else {
            z = rgb.b();
            x = rgb.r() / z;
            y = rgb.g() / z;
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
    std::array<float, 3> coeffs;
    if (zi > SPECTRUM_TABLE_RES - 2) {
        std::cout << "zi out of range, clamping to max" << std::endl;
        zi = SPECTRUM_TABLE_RES - 2;
    }
    float dx = x - xi;
    float dy = y - yi;
    float dz = (z - m_z_nodes[zi]) / (m_z_nodes[zi + 1] - m_z_nodes[zi]);
    for (size_t i = 0; i < 3; i++) {
        auto co = [&](size_t dx, size_t dy, size_t dz) {
            return m_coeffs[maxc][zi + dz][yi + dy][xi + dx][i];
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

    return RGBSigmoidPolynomial(coeffs[0], coeffs[1], coeffs[2]);
}

std::shared_ptr<const RGBToSpectrumTable> RGBToSpectrumTable::sRGB() {
    static std::shared_ptr<const RGBToSpectrumTable> table;
    if (!table) {
        table = std::make_shared<RGBToSpectrumTable>(
            std::array<float, SPECTRUM_TABLE_RES>({
                0, 1.67704457e-06, 2.62230806e-05, 0.000129584747, 0.000399308716, 0.000949404493, 0.00191508455, 0.00344748166, 0.00570843136, 0.00886540301, 0.013086644, 0.0185365994, 0.0253716707, 0.0337363295, 0.0437596664, 0.0555523485, 0.0692040548, 0.0847813785, 0.102326222, 0.121854618, 0.143356144, 0.166793674, 0.192103669, 0.219196871, 0.247959405, 0.2782543, 0.309923291, 0.342789054, 0.376657575, 0.411320895, 0.446559966, 0.482147723, 0.517852247, 0.553440034, 0.588679135, 0.623342395, 0.657210946, 0.690076709, 0.721745729, 0.752040565, 0.780803144, 0.807896316, 0.833206296, 0.856643856, 0.878145397, 0.897673786, 0.915218592, 0.930795968, 0.944447637, 0.956240356, 0.966263652, 0.974628329, 0.981463373, 0.986913383, 0.991134584, 0.994291544, 0.996552527, 0.998084903, 0.999050617, 0.999600708, 0.99987042, 0.999973774, 0.999998331, 1,
            }),
            sRGBToSpectrumTable_Data
        );
    }
    return table;
}

std::shared_ptr<const RGBColorSpace> RGBColorSpace::sRGB() {
    static std::shared_ptr<const RGBColorSpace> space;
    if (!space) {
        space = std::make_shared<RGBColorSpace>(
            Vec2(0.64, 0.33),
            Vec2(.3, 0.6),
            Vec2(0.15, 0.06),
            spectra::STD_ILLUM_D65(),
            RGBToSpectrumTable::sRGB()
        );
    }
    return space;
}