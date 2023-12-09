#pragma once

#include <random>

#include "vec.hpp"

class Sampler {
public:
    Sampler(int samples_per_pixel) : m_samples_per_pixel(samples_per_pixel) {}

    virtual ~Sampler() {}

    virtual float sample_1d() = 0;
    virtual Vec2 sample_2d() = 0;
    
    virtual Vec2 sample_pixel() {
        return sample_2d();
    }

    virtual void start_pixel_sample(int x, int y, int sample_index, int dim) = 0;
    void start_pixel_sample(int x, int y, int sample_index) {
        start_pixel_sample(x, y, sample_index, 0);
    }

    int samples_per_pixel() const {
        return m_samples_per_pixel;
    }

    Vec2 sample_uniform_disk();
    Vec3 sample_uniform_hemisphere();
    float uniform_hemisphere_pdf() {
        return 0.5 * M_1_PI;
    }
    Vec3 sample_uniform_sphere();
    float uniform_sphere_pdf() {
        return 0.25 * M_1_PI;
    }
    Vec3 sample_cosine_hemisphere();
    float cosine_hemisphere_pdf(float cos_theta) {
        return cos_theta * M_1_PI;
    }

protected:
    int m_samples_per_pixel;
};


class BasicSampler : public Sampler {
public:
    explicit BasicSampler(int samples_per_pixel, uint32_t seed) : Sampler(samples_per_pixel), rng(seed), dist(0.0f, 1.0f) {}

    float sample_1d() override;
    Vec2 sample_2d() override;

    void start_pixel_sample(int x, int y, int sample_index, int dim) override;

    std::mt19937 rng;
    std::uniform_real_distribution<float> dist;
};


// used by the Halton sampler
class DigitPermutation {
public:
    DigitPermutation() : base(0), n_digits(0) {};
    DigitPermutation(int base, uint32_t seed);

    int permute(int digitIndex, int digitValue) const;

private:
    int base, n_digits;
    std::vector<uint16_t> permutations;
};


class HaltonSampler : public Sampler {
public:
    HaltonSampler(int samples_per_pixel, int x_res, int y_res, uint32_t seed);

    void start_pixel_sample(int x, int y, int sample_index, int dim);

    float sample_1d() override;
    Vec2 sample_2d() override;
    Vec2 sample_pixel() override;

private:
    float sample_dimension(int dim) const;

    std::vector<DigitPermutation> m_permutations;
    std::array<int64_t, 2> base_scales;
    std::array<int64_t, 2> base_exps;
    std::array<uint64_t, 2> mult_inverse;

    int64_t halton_index = 0;
    int dimension = 0;
};