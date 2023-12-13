#pragma once

#include <memory>

#include "spectrum.hpp"

namespace spectra {

// CIE XYZ

const float CIE_Y_INTEGRAL = 106.856895f;

std::shared_ptr<const DenselySampledSpectrum> X();
std::shared_ptr<const DenselySampledSpectrum> Y();
std::shared_ptr<const DenselySampledSpectrum> Z();

// illuminants

std::shared_ptr<const PiecewiseLinearSpectrum> ILLUM_D65();

// camera sensor spectra

std::shared_ptr<const PiecewiseLinearSpectrum> CANON_EOS_R();
std::shared_ptr<const PiecewiseLinearSpectrum> CANON_EOS_G();
std::shared_ptr<const PiecewiseLinearSpectrum> CANON_EOS_B();

// material IOR and absorption spectra

std::shared_ptr<const PiecewiseLinearSpectrum> AL_IOR();
std::shared_ptr<const PiecewiseLinearSpectrum> AL_ABSORPTION();

std::shared_ptr<const PiecewiseLinearSpectrum> CU_IOR();
std::shared_ptr<const PiecewiseLinearSpectrum> CU_ABSORPTION();

std::shared_ptr<const PiecewiseLinearSpectrum> GLASS_BK7_IOR();

std::shared_ptr<const PiecewiseLinearSpectrum> GLASS_SF11_IOR();

}