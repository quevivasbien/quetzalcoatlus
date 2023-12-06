#include <stdexcept>

#include "spectrum_sample.hpp"

// check that wavelengths match when combining SpectrumSamples
#define CHECK_SPECTRUM_WAVELENGTHS


WavelengthSample::WavelengthSample(
    SampleArray&& lambdas,
    SampleArray&& pdf
) : m_lambdas(std::move(lambdas)), m_pdf(std::move(pdf)) {}

WavelengthSample WavelengthSample::uniform(float u, float lambda_min, float lambda_max) {
    SampleArray lambdas;
    lambdas[0] = (1.0f - u) * lambda_min + u * lambda_max;
    float delta = (lambda_max - lambda_min) / N_SPECTRUM_SAMPLES;
    for (size_t i = 1; i < N_SPECTRUM_SAMPLES; i++) {
        lambdas[i] = lambdas[i - 1] + delta;
        if (lambdas[i] > lambda_max) {
            lambdas[i] = lambda_min + (lambdas[i] - lambda_max);
        }
    }
    SampleArray pdf;
    pdf.fill(1.0f / (lambda_max - lambda_min));
    return WavelengthSample(std::move(lambdas), std::move(pdf));
}

bool WavelengthSample::secondary_terminated() const {
    for (size_t i = 1; i < N_SPECTRUM_SAMPLES; i++) {
        if (m_pdf[i] != 0.0f) {
            return false;
        }
    }
    return true;
}
void WavelengthSample::terminate_secondary() {
    if (secondary_terminated()) {
        return;
    }
    for (size_t i = 1; i < N_SPECTRUM_SAMPLES; i++) {
        m_pdf[i] = 0.0f;
    }
    m_pdf[0] /= N_SPECTRUM_SAMPLES;
}

SpectrumSample SpectrumSample::from_spectrum(
    const Spectrum& spectrum,
    std::shared_ptr<const WavelengthSample> wavelengths
) {
    SampleArray values;
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; i++) {
        values[i] = spectrum(wavelengths->m_lambdas[i]);
    }
    return SpectrumSample(std::move(values), wavelengths);
}

bool SpectrumSample::is_zero() const {
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        if (m_values[i] != 0.0f) {
            return false;
        }
    }
    return true;
}

SpectrumSample SpectrumSample::operator+(const SpectrumSample& other) const {
    #ifdef CHECK_SPECTRUM_WAVELENGTHS
    if (m_wavelengths != other.m_wavelengths) {
        throw std::runtime_error("wavelength mismatch");
    }
    #endif
    SampleArray values;
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        values[i] = m_values[i] + other.m_values[i];
    }
    return SpectrumSample(std::move(values), m_wavelengths);
}
SpectrumSample& SpectrumSample::operator+=(const SpectrumSample& other) {
    #ifdef CHECK_SPECTRUM_WAVELENGTHS
    if (m_wavelengths != other.m_wavelengths) {
        throw std::runtime_error("wavelength mismatch");
    }
    #endif
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        m_values[i] += other.m_values[i];
    }
    return *this;
}
SpectrumSample SpectrumSample::operator-(const SpectrumSample& other) const {
    #ifdef CHECK_SPECTRUM_WAVELENGTHS
    if (m_wavelengths != other.m_wavelengths) {
        throw std::runtime_error("wavelength mismatch");
    }
    #endif
    SampleArray values;
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        values[i] = m_values[i] - other.m_values[i];
    }
    return SpectrumSample(std::move(values), m_wavelengths);
}
SpectrumSample& SpectrumSample::operator-=(const SpectrumSample& other) {
    #ifdef CHECK_SPECTRUM_WAVELENGTHS
    if (m_wavelengths != other.m_wavelengths) {
        throw std::runtime_error("wavelength mismatch");
    }
    #endif
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        m_values[i] -= other.m_values[i];
    }
    return *this;
}
SpectrumSample SpectrumSample::operator*(const SpectrumSample& other) {
    #ifdef CHECK_SPECTRUM_WAVELENGTHS
    if (m_wavelengths != other.m_wavelengths) {
        throw std::runtime_error("wavelength mismatch");
    }
    #endif
    SampleArray values;
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        values[i] = m_values[i] * other.m_values[i];
    }
    return SpectrumSample(std::move(values), m_wavelengths);
}
SpectrumSample& SpectrumSample::operator*=(const SpectrumSample& other) {
    #ifdef CHECK_SPECTRUM_WAVELENGTHS
    if (m_wavelengths != other.m_wavelengths) {
        throw std::runtime_error("wavelength mismatch");
    }
    #endif
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        m_values[i] *= other.m_values[i];
    }
    return *this;
}
SpectrumSample SpectrumSample::operator/(const SpectrumSample& other) const {
    #ifdef CHECK_SPECTRUM_WAVELENGTHS
    if (m_wavelengths != other.m_wavelengths) {
        throw std::runtime_error("wavelength mismatch");
    }
    #endif
    SampleArray values;
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        values[i] = m_values[i] / other.m_values[i];
    }
    return SpectrumSample(std::move(values), m_wavelengths);
}
SpectrumSample& SpectrumSample::operator/=(const SpectrumSample& other) {
    #ifdef CHECK_SPECTRUM_WAVELENGTHS
    if (m_wavelengths != other.m_wavelengths) {
        throw std::runtime_error("wavelength mismatch");
    }
    #endif
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        m_values[i] /= other.m_values[i];
    }
    return *this;
}

float SpectrumSample::average() const {
    float sum = 0.0f;
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        sum += m_values[i];
    }
    return sum / N_SPECTRUM_SAMPLES;
}


// construct new spectrum from wavelengths pdf
SpectrumSample SpectrumSample::new_from_pdf() const {
    auto pdf = m_wavelengths->m_pdf;
    return SpectrumSample(
        std::move(pdf),
        m_wavelengths
    );
}
