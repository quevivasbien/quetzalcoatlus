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

    WavelengthSample() {};

    static WavelengthSample uniform(float u, float lambda_min = LAMBDA_MIN, float lambda_max = LAMBDA_MAX);

    bool secondary_terminated() const;

    void terminate_secondary();

    float operator[](size_t i) const { return m_lambdas[i]; }

    bool operator==(const WavelengthSample& other) const;

    SampleArray m_lambdas;
    SampleArray m_pdf;
};


class SpectrumSample {
public:
    using SampleArray = std::array<float, N_SPECTRUM_SAMPLES>;

    explicit SpectrumSample(const SampleArray& values) : m_values(values) {}
    explicit SpectrumSample(SampleArray&& values) : m_values(std::move(values)) {}
    
    explicit SpectrumSample(float c) {
        m_values.fill(c);
    }

    SpectrumSample() : m_values({}) {}

    static SpectrumSample from_spectrum(const Spectrum& spectrum, const WavelengthSample& wavelengths);

    float operator[](size_t i) const { return m_values[i]; }
    float& operator[](size_t i) { return m_values[i]; }

    bool is_zero() const;

    float max_component() const;

    // arithmetic operators
    SpectrumSample operator+(const SpectrumSample& other) const;
    SpectrumSample& operator+=(const SpectrumSample& other);
    SpectrumSample operator-(const SpectrumSample& other) const;
    SpectrumSample& operator-=(const SpectrumSample& other);
    SpectrumSample operator*(const SpectrumSample& other);
    SpectrumSample& operator*=(const SpectrumSample& other);
    SpectrumSample operator/(const SpectrumSample& other) const;
    SpectrumSample& operator/=(const SpectrumSample& other);

    SpectrumSample operator+(float c) const;
    SpectrumSample& operator+=(float c);
    SpectrumSample operator-(float c) const;
    SpectrumSample& operator-=(float c);
    SpectrumSample operator*(float c) const;
    SpectrumSample& operator*=(float c);
    SpectrumSample operator/(float c) const;
    SpectrumSample& operator/=(float c);

    float average() const;

    // pointwise map
    template <typename F>
    SpectrumSample map(F&& f) const {
        SampleArray values;
        for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
            values[i] = f(m_values[i]);
        }
        return SpectrumSample(std::move(values));
    }

    template <typename F>
    void map_inplace(F&& f) {
        for (size_t i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
            m_values[i] = f(m_values[i]);
        }
    }

    // construct new spectrum from wavelengths pdf
    static SpectrumSample from_wavelengths_pdf(const WavelengthSample& wavelengths);

    SampleArray m_values;
};

