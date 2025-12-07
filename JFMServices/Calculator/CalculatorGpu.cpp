#include "Calculator.h"

namespace Calculator
{
utils::SymbolLoader *CalculatorAll<CalculationParamsGpu>::loader =
    utils::SymbolLoader::Create(CalculatorAll<CalculationParamsGpu>::library_path);

typedef void (*PFN_calculatorGpuCallProc)(const MCInput &input,
                                          std::vector<MCResult> *output,
                                          std::vector<std::vector<double>> pSets);

void CalculatorAll<CalculationParamsGpu>::call(const CalculationParamsGpu &params)
{
    static const PFN_calculatorGpuCallProc calculatorGpuCallProc =
        reinterpret_cast<PFN_calculatorGpuCallProc>(loader->getProcAddress("calculatorGpuCall"));
    JFM_ASSERT(calculatorGpuCallProc != nullptr);

    calculatorGpuCallProc(params.input,
                          params.output,
                          params.pSets);
    JFM_ASSERT("Has to fail : Not implemented !" == 0);
}
} // namespace Calculator
