#pragma once

#include <vector>
#include "JFMIFitting.hpp"

using JFMService::FittingService::MCInput;
using JFMService::FittingService::MCResult;

#ifdef __cplusplus
extern "C"
{
#endif

void calculatorGpuCall(const MCInput &input,
                       std::vector<MCResult> *output,
                       std::vector<std::vector<double>> pSets);

#ifdef __cplusplus
}
#endif
