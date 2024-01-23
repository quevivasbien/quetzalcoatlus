#include "onb.hpp"
#include "phase_function.hpp"
#include "util.hpp"

float PhaseFunction::operator()(float cos_theta) const {
    float denom = 1.0f + g * g - 2.0f * g * cos_theta;
    return 0.25 * M_1_PI * (1.0f - g * g) / (denom * std::sqrt(denom));
}

std::optional<PhaseFunctionSample> PhaseFunction::sample(Vec3 wo, Vec2 u) const {
    float cos_theta;
    if (std::abs(g) < 1e-3f) {
        cos_theta = 1.0f - 2.0f * u.x;
    }
    else {
        float temp = (1.0f - g * g) / (1.0f + g - 2.0f * g * u.x);
        cos_theta = -1.0f / (2.0f * g) * (1.0f + g * g - temp * temp);
    }
    
    float sin_theta = std::sqrt(1.0f - cos_theta * cos_theta);
    float phi = M_2_PI * u.y;
    Vec3 wi = OrthonormalBasis(wo).from_local(spherical_direction(sin_theta, cos_theta, phi));
    
    return PhaseFunctionSample {
        .wi = wi,
        .pdf = operator()(cos_theta)
    };
}