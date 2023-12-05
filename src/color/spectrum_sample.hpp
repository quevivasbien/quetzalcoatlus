#include <array>
#include <cstddef>
#include <memory>

#include "spectrum.hpp"

#define CHECK_SPECTRUM_WAVELENGTHS

template <size_t N>
class WavelengthSample {
public:
    WavelengthSample(
        std::array<float, N>&& lambdas,
        std::array<float, N>&& pdf
    ) : m_lambdas(std::move(lambdas)), m_pdf(std::move(pdf)) {}
    static WavelengthSample uniform(float u, float lambda_min = LAMBDA_MIN, float lambda_max = LAMBDA_MAX) {
        std::array<float, N> lambdas;
        lambdas[0] = (1.0f - u) * lambda_min + u * lambda_max;
        float delta = (lambda_max - lambda_min) / N;
        for (size_t i = 1; i < N; i++) {
            lambdas[i] = lambdas[i - 1] + delta;
            if (lambdas[i] > lambda_max) {
                lambdas[i] = lambda_min + (lambdas[i] - lambda_max);
            }
        }
        std::array<float, N> pdf;
        pdf.fill(1.0f / (lambda_max - lambda_min));
        return WavelengthSample(std::move(lambdas), std::move(pdf));
    }

    bool secondary_terminated() const {
        for (size_t i = 1; i < N; i++) {
            if (m_pdf[i] != 0.0f) {
                return false;
            }
        }
        return true;
    }

    void terminate_secondary() {
        if (secondary_terminated()) {
            return;
        }
        for (size_t i = 1; i < N; i++) {
            m_pdf[i] = 0.0f;
        }
        m_pdf[0] /= N;
    }

    std::array<float, N> m_lambdas;
    std::array<float, N> m_pdf;
};

template <size_t N>
class SpectrumSample {
public:
    SpectrumSample(
        float c, const std::shared_ptr<WavelengthSample<N>>& wavelengths
    ) : m_wavelengths(wavelengths) {
        m_values.fill(c);
    }

    float operator[](size_t i) const { return m_values[i]; }
    float& operator[](size_t i) { return m_values[i]; }

    bool is_zero() const {
        for (size_t i = 0; i < N; ++i) {
            if (m_values[i] != 0.0f) {
                return false;
            }
        }
        return true;
    }

    // arithmetic operators
    SpectrumSample<N> operator+(const SpectrumSample<N>& other) const {
        #ifdef CHECK_SPECTRUM_WAVELENGTHS
        if (m_wavelengths != other.m_wavelengths) {
            throw std::runtime_error("wavelength mismatch");
        }
        #endif
        std::array<float, N> values;
        for (size_t i = 0; i < N; ++i) {
            values[i] = m_values[i] + other.m_values[i];
        }
        return SpectrumSample<N>(values, m_wavelengths);
    }
    SpectrumSample<N>& operator+=(const SpectrumSample<N>& other) {
        #ifdef CHECK_SPECTRUM_WAVELENGTHS
        if (m_wavelengths != other.m_wavelengths) {
            throw std::runtime_error("wavelength mismatch");
        }
        #endif
        for (size_t i = 0; i < N; ++i) {
            m_values[i] += other.m_values[i];
        }
        return *this;
    }
    SpectrumSample<N> operator-(const SpectrumSample<N>& other) const {
        #ifdef CHECK_SPECTRUM_WAVELENGTHS
        if (m_wavelengths != other.m_wavelengths) {
            throw std::runtime_error("wavelength mismatch");
        }
        #endif
        std::array<float, N> values;
        for (size_t i = 0; i < N; ++i) {
            values[i] = m_values[i] - other.m_values[i];
        }
        return SpectrumSample<N>(values, m_wavelengths);
    }
    SpectrumSample<N>& operator-=(const SpectrumSample<N>& other) {
        #ifdef CHECK_SPECTRUM_WAVELENGTHS
        if (m_wavelengths != other.m_wavelengths) {
            throw std::runtime_error("wavelength mismatch");
        }
        #endif
        for (size_t i = 0; i < N; ++i) {
            m_values[i] -= other.m_values[i];
        }
        return *this;
    }
    SpectrumSample<N> operator*(float c) const {
        std::array<float, N> values;
        for (size_t i = 0; i < N; ++i) {
            values[i] = m_values[i] * c;
        }
        return SpectrumSample<N>(values, m_wavelengths);
    }
    SpectrumSample<N>& operator*=(float c) {
        for (size_t i = 0; i < N; ++i) {
            m_values[i] *= c;
        }
        return *this;
    }
    SpectrumSample<N> operator/(float c) const {
        std::array<float, N> values;
        for (size_t i = 0; i < N; ++i) {
            values[i] = m_values[i] / c;
        }
        return SpectrumSample<N>(values, m_wavelengths);
    }
    SpectrumSample<N>& operator/=(float c) {
        for (size_t i = 0; i < N; ++i) {
            m_values[i] /= c;
        }
        return *this;
    }

    // pointwise map
    template <typename F>
    SpectrumSample<N> map(F&& f) const {
        std::array<float, N> values;
        for (size_t i = 0; i < N; ++i) {
            values[i] = f(m_values[i]);
        }
        return SpectrumSample<N>(values, m_lambdas);
    }

    template <typename F>
    void map_inplace(F&& f) {
        for (size_t i = 0; i < N; ++i) {
            m_values[i] = f(m_values[i]);
        }
    }

private:
    std::array<float, N> m_values;
    std::shared_ptr<WavelengthSample> m_wavelengths;
};

