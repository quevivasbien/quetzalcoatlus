#pragma once

#include <memory>
#include <vector>


const int LAMBDA_MIN = 360;
const int LAMBDA_MAX = 830;

class Spectrum {
public:
    virtual ~Spectrum() {};

    virtual float operator()(float lambda) const = 0;

    float integral() const;
    float inner_product(const Spectrum& other) const;
};


class ConstantSpectrum : public Spectrum {
public:
    explicit ConstantSpectrum(float value) : m_value(value) {}
    float operator()(float lambda) const override {
        return m_value;
    }

    float m_value;
};


class DenselySampledSpectrum : public Spectrum {
public:
    explicit DenselySampledSpectrum(
        std::vector<float>&& values,
        int lambda_min = LAMBDA_MIN
    );
    explicit DenselySampledSpectrum(
        const Spectrum& other,
        int lambda_min = LAMBDA_MIN,
        int lambda_max = LAMBDA_MAX
    );

    float operator()(float lambda) const override;

    float lambda_min() const {
        return m_lambda_min;
    }

    float lambda_max() const {
        return m_lambda_max;
    }

    const std::vector<float>& values() const {
        return m_values;
    }
    
private:
    int m_lambda_min;
    int m_lambda_max;
    std::vector<float> m_values;
};


class PiecewiseLinearSpectrum : public Spectrum {
public:
    PiecewiseLinearSpectrum(
        std::vector<float>&& lambdas,
        std::vector<float>&& values
    );
    static PiecewiseLinearSpectrum from_interleaved(
        const std::vector<float>& interleaved,
        bool normalize = true
    );

    float operator()(float lambda) const override;

private:
    std::vector<float> m_lambdas;
    std::vector<float> m_values;
};


class BlackbodySpectrum : public Spectrum {
public:
    explicit BlackbodySpectrum(float t);

    float operator()(float lambda) const override;

private:
    float m_t;
    float m_normalization_factor;
};
