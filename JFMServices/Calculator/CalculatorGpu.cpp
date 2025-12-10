#include "Calculator.h"
#include "CalculatorGpu/CalculatorGpu.h"
#include "JFMAdditionalParameters.hpp"

namespace
{
static const CudaParams defaultCalculationParams = {
    100,     /* blocks per grid */
    256,     /* threads per block*/
    512,    /* memory pool size */
};
} // namespace anonymous

namespace Calculator
{
utils::SymbolLoader *CalculatorAll<CalculationParamsGpu>::loader =
    utils::SymbolLoader::Create(CalculatorAll<CalculationParamsGpu>::library_path);

typedef void (*PFN_calculatorGpuCallProc)(CudaParams calculationParams,
                                          const MCInput &input,
                                          std::vector<MCResult> *output,
                                          size_t totalLength,
                                          double Temperature,
                                          double Noise);

void CalculatorAll<CalculationParamsGpu>::call(const CalculationParamsGpu &params)
{
    static const PFN_calculatorGpuCallProc calculatorGpuCallProc =
        reinterpret_cast<PFN_calculatorGpuCallProc>(loader->getProcAddress("calculatorGpuCall"));
    JFM_ASSERT(calculatorGpuCallProc != nullptr);

    CudaParams calcParams = ::defaultCalculationParams;

    fillUpResultOutputs(params.input, params.output, params.cartesian);
    double Temperature = params.input.startingData.initialData.getTemperature();

    calculatorGpuCallProc(calcParams,
                          params.input,
                          params.output,
                          params.output->size(),
                          Temperature,
                          params.input.noise);
}
} // namespace Calculator
