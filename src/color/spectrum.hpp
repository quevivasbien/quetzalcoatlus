#include <vector>

const int LAMBDA_MIN = 360;
const int LAMBDA_MAX = 830;

class Spectrum {
public:
    virtual float operator()(float lambda) const = 0;
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


class BlackbodySpectrum : public Spectrum {
public:
    explicit BlackbodySpectrum(float t);

    float operator()(float lambda) const override;

private:
    float m_t;
    float m_normalization_factor;
};


namespace spectra {
    const DenselySampledSpectrum& X();
    const DenselySampledSpectrum& Y();
    const DenselySampledSpectrum& Z();
}