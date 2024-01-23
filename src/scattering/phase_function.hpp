#pragma once

#include <cmath>
#include <optional>

#include "vec.hpp"

struct PhaseFunctionSample {
    Vec3 wi;
    float pdf;
};

// A Henyey-Greenstein phase function
// Models a distribution of scattering angles
class PhaseFunction {
public:
    explicit PhaseFunction(float g) : g(g) {}

    // calculate probability density of scattering between wo and wi
    // wo and wi are assumed to be normalized
    float operator()(Vec3 wo, Vec3 wi) const {
        return operator()(wo.dot(wi));
    }

    // calculate probability density of scattering with cosine angle cos_theta
    float operator()(float cos_theta) const;

    // sample wi given wo
    std::optional<PhaseFunctionSample> sample(Vec3 wo, Vec2 u) const;

private:
    float g;
};