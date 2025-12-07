#include <cuda_runtime.h>
#include "macros.h"
#include "CalculatorGpu.h"

namespace
{
__global__ static void addVector(const float *A, const float *B, float *C, int numElements)
{
    int idx = blockDim.x * blockIdx.x + threadIdx.x;

    if (idx < numElements)
        C[idx] = A[idx] + B[idx];
}
} // namespace anonymous

void calculatorGpuCall(
            const MCInput &input,
            std::vector<MCResult> *output,
            std::vector<std::vector<double>> pSets)
{
    addVector<<<1,1,1>>>(NULL, NULL, NULL, 1);
    JFM_ASSERT("Has to fail : Not implemented !" == 0);
}
