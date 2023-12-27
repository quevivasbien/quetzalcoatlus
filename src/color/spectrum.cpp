#include <algorithm>
#include <cmath>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <vector>
        
#include "spectra.hpp"
#include "spectrum.hpp"


float Spectrum::integral() const {
    float sum = 0.0f;
    for (int l = LAMBDA_MIN; l <= LAMBDA_MAX; ++l) {
        sum += (*this)(l);
    }
    return sum;
}

float Spectrum::inner_product(const Spectrum& other) const {
    float sum = 0.0f;
    for (int l = LAMBDA_MIN; l <= LAMBDA_MAX; ++l) {
        sum += (*this)(l) * other(l);
    }
    return sum;
}


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
    long index = std::lroundf(lambda - m_lambda_min);
    if (index < 0 || index >= m_values.size()) {
        return 0.0f;
    }
    return m_values[index];
}


PiecewiseLinearSpectrum::PiecewiseLinearSpectrum(
    std::vector<float>&& lambdas,
    std::vector<float>&& values
) : m_lambdas(std::move(lambdas)), m_values(std::move(values)) {
    if (m_lambdas.size() != m_values.size()) {
        throw std::runtime_error("m_lambdas.size() != m_values.size()");
    }
}

PiecewiseLinearSpectrum PiecewiseLinearSpectrum::from_interleaved(
    const std::vector<float>& interleaved,
    bool normalize
) {
    if (interleaved.size() % 2 != 0) {
        throw std::runtime_error("interleaved.size() % 2 != 0");
    }
    if (interleaved.size() == 0) {
        throw std::runtime_error("interleaved.size() == 0");
    }
    std::vector<float> lambdas;
    lambdas.reserve(interleaved.size() / 2 + 2);
    std::vector<float> values;
    values.reserve(interleaved.size() / 2 + 2);

    if (interleaved[0] > LAMBDA_MIN) {
        lambdas.push_back(LAMBDA_MIN - 1);
        values.push_back(interleaved[1]);
    }
    for (size_t i = 0; i < interleaved.size(); i += 2) {
        lambdas.push_back(interleaved[i]);
        values.push_back(interleaved[i + 1]);
    }
    if (interleaved.back() < LAMBDA_MAX) {
        lambdas.push_back(LAMBDA_MAX + 1);
        values.push_back(interleaved.back());
    }
    PiecewiseLinearSpectrum spec(std::move(lambdas), std::move(values));
    if (normalize) {
        float c = spec.inner_product(*spectra::Y());
        std::transform(spec.m_values.begin(), spec.m_values.end(), spec.m_values.begin(), [c](float v) { return v * spectra::CIE_Y_INTEGRAL / c; });
    }
    
    return spec;
}

float PiecewiseLinearSpectrum::operator()(float lambda) const {
    if (m_lambdas.empty() || lambda < m_lambdas.front() || lambda > m_lambdas.back()) {
        return 0.0f;
    }
    auto pp = std::lower_bound(m_lambdas.begin(), m_lambdas.end(), lambda);
    size_t i = std::distance(m_lambdas.begin(), pp);
    float t = (lambda - m_lambdas[i]) / (m_lambdas[i + 1] - m_lambdas[i]);
    return m_values[i] * (1.0f - t) + m_values[i + 1] * t;
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
