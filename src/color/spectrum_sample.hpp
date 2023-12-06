#pragma once

#include <array>
#include <cstddef>
#include <memory>

#include "spectrum.hpp"

const size_t N_SPECTRUM_SAMPLES = 4;

class WavelengthSample {
public:
    using SampleArray = std::array<float, N_SPECTRUM_SAMPLES>;

    WavelengthSample(
        SampleArray&& lambdas,
        SampleArray&& pdf
    );

    static WavelengthSample uniform(float u, float lambda_min = LAMBDA_MIN, float lambda_max = LAMBDA_MAX);

    bool secondary_terminated() const;

    void terminate_secondary();

    SampleArray m_lambdas;
    SampleArray m_pdf;
};


class SpectrumSample {
public:
    using SampleArray = std::array<float, N_SPECTRUM_SAMPLES>;

    SpectrumSample(
        SampleArray&& values,
        const std::shared_ptr<const WavelengthSample>& wavelengths
    ) : m_values(std::move(values)), m_wavelengths(wavelengths) {}
    
    SpectrumSample(
        float c, const std::shared_ptr<const WavelengthSample>& wavelengths
    ) : m_wavelengths(wavelengths) {
        m_values.fill(c);
    }

    static SpectrumSample from_spectrum(const Spectrum& spectrum, std::shared_ptr<const WavelengthSample> wavelengths);

    float operator[](size_t i) const { return m_values[i]; }
    float& operator[](size_t i) { return m_values[i]; }

    bool is_zero() const;

    // arithmetic operators
    SpectrumSample operator+(const SpectrumSample& other) const;
    SpectrumSample& operator+=(const SpectrumSample& other);
    SpectrumSample operator-(const SpectrumSample& other) const;
    SpectrumSample& operator-=(const SpectrumSample& other);
    SpectrumSample operator*(const SpectrumSample& other);
    SpectrumSample& operator*=(const SpectrumSample& other);
    SpectrumSample operator/(const SpectrumSample& other) const;
    SpectrumSample& operator/=(const SpectrumSample& other);

    float average() const;

    // pointwise map
    template <typename F>
    SpectrumSample map(F&& f) const {
        SampleArray values;
        for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
            values[i] = f(m_values[i]);
        }
        return SpectrumSample(values, m_wavelengths);
    }

    template <typename F>
    void map_inplace(F&& f) {
        for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
            m_values[i] = f(m_values[i]);
        }
    }

    // construct new spectrum from wavelengths pdf
    SpectrumSample new_from_pdf() const;

    SampleArray m_values;
    std::shared_ptr<const WavelengthSample> m_wavelengths;
};

