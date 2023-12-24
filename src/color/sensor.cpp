#include <algorithm>

#include "sensor.hpp"

const float SENSOR_SATURATION = 40.0f;

const Mat3 LMS_FROM_XYZ = Mat3({
    0.8951,  0.2664, -0.1614,
    -0.7502, 1.7135,  0.0367,
    0.0389, -0.0685,  1.0296
});

const Mat3 XYZ_FROM_LMS = Mat3({
    0.986993,   -0.147054,  0.159963,
    0.432305,    0.51836,   0.0492912,
    -0.00852866, 0.0400428, 0.968487
});

Mat3 white_balance(Vec2 source_white, Vec2 target_white) {
    auto source_xyz = XYZ::from_xyY(source_white.x, source_white.y);
    auto target_xyz = XYZ::from_xyY(target_white.x, target_white.y);
    auto source_lms = LMS_FROM_XYZ * source_xyz;
    auto target_lms = LMS_FROM_XYZ * target_xyz;

    auto lms_correct = Mat3::diagonal({
        target_lms.x / source_lms.x,
        target_lms.y / source_lms.y,
        target_lms.z / source_lms.z
    });
    return LMS_FROM_XYZ * lms_correct * LMS_FROM_XYZ;
}

PixelSensor::PixelSensor(
    const RGBColorSpace& cs,
    const Spectrum& illuminant,
    float imaging_ratio
) : m_r(*spectra::X()), m_g(*spectra::Y()), m_b(*spectra::Z()), m_imaging_ratio(imaging_ratio) {
    auto source_white = XYZ::from_spectrum(illuminant).xy();
    auto target_white = cs.whitepoint();
    m_xyz_from_sensor_rgb = white_balance(source_white, target_white);
}

PixelSensor::PixelSensor(
    const Spectrum& r,
    const Spectrum& g,
    const Spectrum& b,
    const RGBColorSpace& cs,
    const Spectrum& illuminant,
    float imaging_ratio
) : m_r(r), m_g(g), m_b(b), m_imaging_ratio(imaging_ratio) {
    // TODO: figure out how to do color correction like in pbrt
    auto source_white = XYZ::from_spectrum(illuminant).xy();
    auto target_white = cs.whitepoint();
    m_xyz_from_sensor_rgb = white_balance(source_white, target_white);
}

RGB PixelSensor::to_sensor_rgb(const SpectrumSample& sample, const WavelengthSample& wavelengths) const {
    auto l = sample / SpectrumSample::from_wavelengths_pdf(wavelengths);
    RGB rgb(
        (SpectrumSample::from_spectrum(m_r, wavelengths) * l).average() * m_imaging_ratio,
        (SpectrumSample::from_spectrum(m_g, wavelengths) * l).average() * m_imaging_ratio,
        (SpectrumSample::from_spectrum(m_b, wavelengths) * l).average() * m_imaging_ratio
    );
    // clamp total contribution to avoid super bright speckles
    float m = std::max({rgb.x, rgb.y, rgb.z});
    if (m > SENSOR_SATURATION) {
        rgb *= SENSOR_SATURATION / m;
    }
    return rgb;
}

PixelSensor PixelSensor::CIE_XYZ(float imaging_ratio) {
    return PixelSensor(
        *RGBColorSpace::sRGB(),
        *spectra::ILLUM_D65(),
        imaging_ratio
    );
}

PixelSensor PixelSensor::CANON_EOS(float imaging_ratio) {
    return PixelSensor(
        *spectra::CANON_EOS_R(),
        *spectra::CANON_EOS_G(),
        *spectra::CANON_EOS_B(),
        *RGBColorSpace::sRGB(),
        *spectra::ILLUM_D65(),
        imaging_ratio
    );
}
