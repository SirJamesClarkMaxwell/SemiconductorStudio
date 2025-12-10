#pragma once

#include <vector>
#include "JFMIFitting.hpp"

using JFMService::FittingService::MCInput;
using JFMService::FittingService::MCResult;

struct CudaParams {
    int blocksPerGrid;
    int threadsPerBlock;
    // [MB] maximum amount of device memory used for a single kernel execution
    size_t memoryPoolSize;
};

#ifdef __cplusplus
extern "C"
{
#endif

void calculatorGpuCall(CudaParams calculationParams,
                       const MCInput &input,
                       std::vector<MCResult> *output,
                       size_t totalLength,
                       double Temperature,
                       double Noise);

#ifdef __cplusplus
}
#endif
