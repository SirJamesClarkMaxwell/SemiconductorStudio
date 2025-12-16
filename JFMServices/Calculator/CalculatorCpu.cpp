#include "Calculator.h"
#include "MonteCarloEngine.hpp"
#include <thread>

namespace Calculator
{
void calculateFittingErrorByBatch(
    const MCInput &input,
    std::vector<MCResult> *output,
    const ProductT &cartesian,
    size_t startIndx,
    size_t length,
    SimulationEngine *simulationEngine)
{
    for (size_t currIndx = 0; currIndx < length; ++currIndx)
    {
        size_t indx = startIndx + currIndx;
        auto&&t = cartesian[indx];
        MCResult& result = output->at(indx);

        ParameterMap& param = result.foundParameters;
        param[0] = std::get<0>(t);
        param[1] = std::get<1>(t);
        param[2] = std::get<2>(t);
        param[3] = std::get<3>(t);

        if (simulationEngine != nullptr)
            simulationEngine->CalculateError(input, result);
    }
}

void fillUpResultOutputs(
    const MCInput &input,
    std::vector<MCResult> *output,
    const ProductT &cartesian)
{
    calculateFittingErrorByBatch(input, output, cartesian, 0, static_cast<size_t>(cartesian.size()), nullptr);
}

void CalculatorAll<CalculationParamsCpuSingle>::call(const CalculationParamsCpuSingle &params)
{
    MEASURE_TIME_THIS_FUNC(
    calculateFittingErrorByBatch(params.input,
                                 params.output,
                                 params.cartesian,
                                 0,
                                 params.totalLength,
                                 params.engine);
    );
}

void CalculatorAll<CalculationParamsCpuMulti>::call(const CalculationParamsCpuMulti &params)
{
    const MCInput &input = params.input;
    std::vector<MCResult> *output = params.output;
    size_t totalLength = params.totalLength;
    unsigned threadCount = std::thread::hardware_concurrency();
    auto cartesian = params.cartesian;
    Info() << "Multithreaded mode - using all threads : " << threadCount << "\n";
    std::vector<std::thread> threads(threadCount);
    const size_t batchLength = totalLength / threadCount;
    const size_t batchLengthReminder = totalLength % threadCount;

    Info() << "------------------------------------------------------\n";
    Info() << "Multihreaded mode:\n";
    Info() << "\tnumber of threads : " << threadCount << "\n";
    Info() << "------------------------------------------------------\n";

    for (uint32_t threadIndx = 0; threadIndx < threadCount; ++threadIndx)
    {
        size_t startIndx = threadIndx * batchLength;
        size_t length = batchLength +
            (threadIndx+1 != threadCount ? 0 : batchLengthReminder);

        threads[threadIndx] = std::thread(calculateFittingErrorByBatch,
                                input, output, cartesian, startIndx, length, params.engine);
    }
    for (auto &t : threads)
        t.join();
}

void CalculatorAll<CalculationParamsSimulate>::call(const CalculationParamsSimulate &params)
{
    const MCInput &input = params.input;
    std::vector<MCResult> *outputs = params.output;
    JFM_ASSERT(outputs->size() == input.iterations);
    std::shared_ptr<Fitters::AbstractFitter> fitter = params.fitter;
    std::shared_ptr<AbstractPreFit> preFitter = params.preFitter;
    auto *monteCarloEngine = reinterpret_cast<MonteCarloEngine *>(params.engine);

    Info() << "------------------------------------------------------\n";
    Info() << "Simulation:\n";
    Info() << "\titerations : " << input.iterations << "\n";
    Info() << "------------------------------------------------------\n";

    MEASURE_TIME("simulate",
    for (size_t iteration = 0; iteration < input.iterations; ++iteration)
    {
        monteCarloEngine->simulateSingleIteration(preFitter,
                                                  fitter,
                                                  const_cast<MCInput &>(input),
                                                  outputs->at(iteration));
    }
    );
}
} // namespace Calculator
