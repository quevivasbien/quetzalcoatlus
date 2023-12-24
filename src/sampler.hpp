#pragma once

#include <random>

#include "vec.hpp"

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


// a Halton sampler, for deterministically generating 1d and 2d values
class Sampler {
public:
    Sampler(int samples_per_pixel, int x_res, int y_res, uint32_t seed);

    float sample_1d();
    Vec2 sample_2d();
    Vec2 sample_pixel();

    void start_pixel_sample(int x, int y, int sample_index, int dim);
    void start_pixel_sample(int x, int y, int sample_index) {
        start_pixel_sample(x, y, sample_index, 0);
    }

    int samples_per_pixel() const {
        return m_samples_per_pixel;
    }

    static Vec2 sample_uniform_disk(Vec2 uv);
    Vec2 sample_uniform_disk();
    static Vec2 sample_uniform_disk_polar(Vec2 uv);
    Vec2 sample_uniform_disk_polar();
    static Vec3 sample_uniform_hemisphere(Vec2 uv);
    Vec3 sample_uniform_hemisphere();
    static float uniform_hemisphere_pdf() {
        return 0.5 * M_1_PI;
    }
    static Vec3 sample_uniform_sphere(Vec2 uv);
    Vec3 sample_uniform_sphere();
    static float uniform_sphere_pdf() {
        return 0.25 * M_1_PI;
    }
    static Vec3 sample_cosine_hemisphere(Vec2 uv);
    Vec3 sample_cosine_hemisphere();
    static float cosine_hemisphere_pdf(float cos_theta) {
        return cos_theta * M_1_PI;
    }

    // sample from a distribution on [0, 1] whose pdf is lerp(a, b, x)
    // not that this is a nonuniform sample over [0, 1], *not* a uniform sample over [a, b]
    // float sample_linear(float a, float b);
    // static float linear_pdf(float x, float a, float b);
    // // same idea as sample_linear, but for a bilinear patch
    // Vec2 sample_bilinear(const std::array<float, 4>& w);
    // static float bilinear_pdf(const Vec2& uv, const std::array<float, 4>& w);

private:
    float sample_dimension(int dim) const;

    std::vector<DigitPermutation> m_permutations;
    std::array<int64_t, 2> base_scales;
    std::array<int64_t, 2> base_exps;
    std::array<uint64_t, 2> mult_inverse;

    int m_samples_per_pixel;
    int64_t halton_index = 0;
    int dimension = 0;
};
