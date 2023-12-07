#pragma once

#include <memory>

#include "spectrum.hpp"

namespace spectra {

const float CIE_Y_INTEGRAL = 106.856895f;

std::shared_ptr<const DenselySampledSpectrum> X();
std::shared_ptr<const DenselySampledSpectrum> Y();
std::shared_ptr<const DenselySampledSpectrum> Z();

std::shared_ptr<const PiecewiseLinearSpectrum> ILLUM_D65();

std::shared_ptr<const PiecewiseLinearSpectrum> CANON_EOS_R();
std::shared_ptr<const PiecewiseLinearSpectrum> CANON_EOS_G();
std::shared_ptr<const PiecewiseLinearSpectrum> CANON_EOS_B();

}