#include "rgb.hpp"

RGBColorSpace::RGBColorSpace(
    Vec2 r, Vec2 g, Vec2 b,
    const std::shared_ptr<Spectrum>& illuminant,
    const std::shared_ptr<RGBToSpectrumTable>& table
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

RGB RGBColorSpace::from_sample(const SpectrumSample<3>& ss) const {
    return from_xyz(XYZ::from_sample(ss));
}
