#include <cassert>

#include "spectrum_sample.hpp"


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

bool WavelengthSample::operator==(const WavelengthSample& other) const {
    return m_lambdas == other.m_lambdas && m_pdf == other.m_pdf;
}


SpectrumSample SpectrumSample::from_spectrum(
    const Spectrum& spectrum,
    const WavelengthSample& wavelengths
) {
    SampleArray values;
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; i++) {
        values[i] = spectrum(wavelengths.m_lambdas[i]);
    }
    return SpectrumSample(std::move(values));
}

bool SpectrumSample::is_zero() const {
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        if (m_values[i] != 0.0f) {
            return false;
        }
    }
    return true;
}

float SpectrumSample::max_component() const {
    float max = m_values[0];
    for (size_t i = 1; i < N_SPECTRUM_SAMPLES; ++i) {
        if (m_values[i] > max) {
            max = m_values[i];
        }
    }
    return max;
}


SpectrumSample SpectrumSample::operator+(const SpectrumSample& other) const {
    SampleArray values;
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        values[i] = m_values[i] + other.m_values[i];
    }
    return SpectrumSample(std::move(values));
}
SpectrumSample& SpectrumSample::operator+=(const SpectrumSample& other) {
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        m_values[i] += other.m_values[i];
    }
    return *this;
}
SpectrumSample SpectrumSample::operator-(const SpectrumSample& other) const {
    SampleArray values;
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        values[i] = m_values[i] - other.m_values[i];
    }
    return SpectrumSample(std::move(values));
}
SpectrumSample& SpectrumSample::operator-=(const SpectrumSample& other) {
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        m_values[i] -= other.m_values[i];
    }
    return *this;
}
SpectrumSample SpectrumSample::operator*(const SpectrumSample& other) const {
    SampleArray values;
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        values[i] = m_values[i] * other.m_values[i];
    }
    return SpectrumSample(std::move(values));
}
SpectrumSample& SpectrumSample::operator*=(const SpectrumSample& other) {
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        m_values[i] *= other.m_values[i];
    }
    return *this;
}
SpectrumSample SpectrumSample::operator/(const SpectrumSample& other) const {
    SampleArray values;
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        values[i] = other.m_values[i] == 0.0f ? 0.0f : m_values[i] / other.m_values[i];
    }
    return SpectrumSample(std::move(values));
}
SpectrumSample& SpectrumSample::operator/=(const SpectrumSample& other) {
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        if (other.m_values[i] == 0.0f) {
            m_values[i] = 0.0f;
        }
        else {
            m_values[i] /= other.m_values[i];
        }
    }
    return *this;
}

SpectrumSample SpectrumSample::operator+(float c) const {
    SampleArray values;
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        values[i] = m_values[i] + c;
    }
    return SpectrumSample(std::move(values));
}

SpectrumSample& SpectrumSample::operator+=(float c) {
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        m_values[i] += c;
    }
    return *this;
}

SpectrumSample SpectrumSample::operator-(float c) const {
    SampleArray values;
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        values[i] = m_values[i] - c;
    }
    return SpectrumSample(std::move(values));
}

SpectrumSample& SpectrumSample::operator-=(float c) {
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        m_values[i] -= c;
    }
    return *this;
}

SpectrumSample SpectrumSample::operator*(float c) const {
    SampleArray values;
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        values[i] = m_values[i] * c;
    }
    return SpectrumSample(std::move(values));
}

SpectrumSample& SpectrumSample::operator*=(float c) {
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        m_values[i] *= c;
    }
    return *this;
}

SpectrumSample SpectrumSample::operator/(float c) const {
    SampleArray values;
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        values[i] = c == 0.0f ? 0.0f : m_values[i] / c;
    }
    return SpectrumSample(std::move(values));
}

SpectrumSample& SpectrumSample::operator/=(float c) {
    for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        if (c == 0.0f) {
            m_values[i] = 0.0f;
        }
        else {
            m_values[i] /= c;
        }
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
SpectrumSample SpectrumSample::from_wavelengths_pdf(const WavelengthSample& wavelengths) {
    return SpectrumSample(wavelengths.m_pdf);
}
