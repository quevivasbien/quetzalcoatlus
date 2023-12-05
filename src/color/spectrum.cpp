#include <cmath>
#include <memory>
#include <vector>
        
#include "spectrum.hpp"


DenselySampledSpectrum::DenselySampledSpectrum(
    std::vector<float> &&values,
    int lambda_min
) : m_lambda_min(lambda_min), m_values(std::move(values)) {
    m_lambda_max = m_lambda_min + m_values.size() - 1;
}

DenselySampledSpectrum::DenselySampledSpectrum(
    const Spectrum& other,
    int lambda_min,
    int lambda_max
) : m_lambda_min(lambda_min), m_lambda_max(lambda_max) {
    m_values.resize(lambda_max - lambda_min + 1);
    for (int i = 0; i < m_values.size(); ++i) {
        m_values[i] = other(lambda_min + i);
    }
}

float DenselySampledSpectrum::operator()(float lambda) const {
    long index = std::lroundf(lambda);
    if (index < 0 || index >= m_values.size()) {
        return 0.0f;
    }
    return m_values[index];
}

// gets blackbody emission in W/m^2; lambda in nm, t in Kelvin
float blackbody(float lambda, float t) {
    if (t <= 0.f) {
        return 0.f;
    }
    const float c = 299792458.f;
    const float h = 6.62606957e-34f;
    const float kb = 1.3806488e-23f;
    float l = lambda * 1e-9f;
    float le = 2.0f * h * c * c /
        (std::pow(lambda, 5) * (std::expm1f(h * c / (l * kb * t))));
    return le;
}

BlackbodySpectrum::BlackbodySpectrum(float t) : m_t(t) {
    float lambda_max = 2.897721e-12f / t;
    m_normalization_factor = 1.0f / blackbody(lambda_max, t);
}

float BlackbodySpectrum::operator()(float lambda) const {
    return m_normalization_factor * blackbody(lambda, m_t);
}
