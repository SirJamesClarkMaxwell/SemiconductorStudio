#include "MonteCarloEngine.hpp"
#include "../Models/JFMErrorModel.hpp"
#include "../Fitting/JFMFitter.hpp"
#include "../Models/CalculateData.hpp"
#include <utils.hpp>
#include <compare>
#include <thread>
#include <future>

namespace JFMService
{
    // Simple, single consistent implementation: remove duplicate/invalid code and
    // use class members from the header (m_generator, m_fitter, m_prefitter).

    MonteCarloEngine::MonteCarloEngine() = default;

    void MonteCarloEngine::generateNoise(double& value, double factor)
    {
        double noise = (value * factor / 100.0);
        std::normal_distribution<double> distribution{0.0, 1.0};
        value += distribution(m_generator) * noise;
    }

    void MonteCarloEngine::simulateChunk(int startIdx, int chunkSize, const std::shared_ptr<AbstractPreFit>& preFitter,
                                         const std::shared_ptr<Fitters::AbstractFitter> fitter, MCInput& input,
                                         std::vector<MCResult>& localResults, int /*numBlock*/)
    {
        for (int i = 0; i < chunkSize; ++i)
        {
            int idx = startIdx + i;
            if (idx >= input.iterations)
            {
                localResults.resize(i);
                break;
            }
            simulate(preFitter, fitter, input, localResults, i);
        }
    }

    void MonteCarloEngine::Simulate(const MCInput& input, std::function<void(MCOutput&&)> callback)
    {
        // protect against zero iterations
        if (input.iterations <= 0)
        {
            if (callback)
            {
                MCOutput out; out.inputData = input; callback(std::move(out));
            }
            return;
        }

        int chunkSize = (input.iterations / 12) > 1 ? static_cast<int>(input.iterations / 12) : 1;

        std::thread worker([this, input, callback, chunkSize]() mutable {
            MCOutput output;
            output.inputData = input;
            // obtain fitter/prefitter for selected model
            auto fitter = m_fitter[input.startingData.initialData.modelID];
            auto preFitter = m_prefitter[input.startingData.initialData.modelID];

            auto start = std::chrono::high_resolution_clock::now();
            output.mcResult.resize(input.iterations);

            std::vector<MCResult> finalResults(input.iterations);

            std::vector<std::future<std::vector<MCResult>>> futures;
            int numChunks = (input.iterations + chunkSize - 1) / chunkSize;
            for (int chunk = 0; chunk < numChunks; ++chunk)
            {
                int startIdx = chunk * chunkSize;
                futures.push_back(std::async(std::launch::async,
                    [this, startIdx, chunkSize, input, preFitter, fitter]() {
                        std::vector<MCResult> localResults(chunkSize);
                        // local copy of input will be passed to simulate; simulate will use provided input copy
                        MCInput localInput = input;
                        simulateChunk(startIdx, chunkSize, preFitter, fitter, localInput, localResults, 0);
                        return localResults;
                    }));
            }

            for (int chunk = 0; chunk < numChunks; ++chunk)
            {
                auto localResults = futures[chunk].get();
                std::copy(localResults.begin(), localResults.end(), output.mcResult.begin() + (chunk * chunkSize));
            }

            auto end = std::chrono::high_resolution_clock::now();
            auto miliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            auto perIteration = miliseconds / input.iterations;
            auto seconds = miliseconds / 1000;
            std::cout << "time: " << seconds << " s  (" << perIteration << " ms per fit)" << std::endl;

            if (callback)
                callback(std::move(output));
        });

        worker.detach();
    }

    void MonteCarloEngine::simulate(const std::shared_ptr<AbstractPreFit>& preFitter, const std::shared_ptr<Fitters::AbstractFitter> fitter, MCInput& input, std::vector<MCResult>& results, int i)
    {
        MCResult result;
        MCInput copied{ input };
        auto characteristic = input.startingData.initialData.characteristic;
        std::vector<double> copiedCurrent{ characteristic.currentData.begin(), characteristic.currentData.end() };
        std::vector<double> copiedVoltage{ characteristic.voltageData.begin(), characteristic.voltageData.end() };
        copied.startingData.initialData.characteristic.currentData = { copiedCurrent.begin(), copiedCurrent.end() };

        auto outOfBounds = [](const ParameterMap& PMap, const ParamBounds& bounds)
        {
            for (const auto& kv : PMap)
            {
                const auto &key = kv.first;
                const auto &val = kv.second;
                if (val < bounds.at(key).first || val > bounds.at(key).second)
                    return true;
            }
            return false;
        };

        auto callback = [&](const ParameterMap&& fittingResult)
        { result.foundParameters = fittingResult; };

        std::vector<double> calculated;
        do
        {
            copiedCurrent = { characteristic.currentData.begin(), characteristic.currentData.end() };
            copied.startingData.initialData.characteristic.currentData = { copiedCurrent.begin(), copiedCurrent.end() };
            for (auto& I : copiedCurrent)
                generateNoise(I, copied.noise);

            copied.startingData.initialValues = preFitter->Estimate(copied.startingData.initialData);
            fitter->Fit(copied.startingData, callback);
            calculateFittingError(input, result, calculated);
        } while (result.error > 23.5 || outOfBounds(result.foundParameters, input.startingData.bounds));

        if ((size_t)i < results.size())
            results[i] = result;

        std::cout << "iteration: " << i << std::endl;
    }

    void MonteCarloEngine::calculateFittingError(const MCInput& input, MCResult& result, std::vector<double>& calculated)
    {
        CalculatingData data;
        DataCalculator dataCalculator;

        Chi2ErrorModel errorModel;
        auto characteristic = input.startingData.initialData.characteristic;
        std::vector<double> copiedCurrent{ characteristic.currentData.begin(), characteristic.currentData.end() };
        std::vector<double> copiedVoltage{ characteristic.voltageData.begin(), characteristic.voltageData.end() };

        data.additionalParameters = input.startingData.initialData.additionalParameters;
        data.parameters = result.foundParameters;
        data.characteristic = { {copiedVoltage.begin(), copiedVoltage.end()}, {copiedCurrent.begin(), copiedCurrent.end()} };
        data.modelID = input.startingData.initialData.modelID;

        std::vector<double> trueCurrentVector(copiedCurrent.size());
        CalculatingData trueData = data;
        trueData.parameters = input.trueParameters;
        trueData.characteristic.currentData = std::span<double>{ trueCurrentVector };

        dataCalculator.CalculateData(data);
        dataCalculator.CalculateData(trueData);
        calculated = copiedCurrent;

        std::span<double> trueCurrent{ trueCurrentVector };
        std::span<double> fittedCurrent = data.characteristic.currentData;
        double accumulatedError = 0.0;
        double noise = input.noise / 100.0;

        auto IerrorModel = [&](double trueI, double fittedI)
        {
            return std::pow(((std::log(fittedI) - std::log(trueI)) / (noise)), 2);
        };

        for (const auto& pair : std::views::zip(trueData.characteristic.currentData, fittedCurrent))
        {
            double trueI = std::get<0>(pair);
            double fitI = std::get<1>(pair);
            accumulatedError += IerrorModel(trueI, fitI);
        }

        result.error = accumulatedError;
    }

    double MonteCarloEngine::GetUncertainty(const MCOutput& output, int level, ParameterID id)
    {
        int degreesOfFreedom = calculateDegreesOfFreedom(output.inputData.trueParameters, output.inputData.startingData.fixConfig);

        std::vector<MCResult> resultsCpy{ output.mcResult };

        auto maxIt = std::max_element(resultsCpy.begin(), resultsCpy.end(),
            [&](const auto& lhs, const auto& rhs) {
                return lhs.foundParameters.at(id) < rhs.foundParameters.at(id);
            });

        auto minIt = std::min_element(resultsCpy.begin(), resultsCpy.end(),
            [&](const auto& lhs, const auto& rhs) {
                return lhs.foundParameters.at(id) < rhs.foundParameters.at(id);
            });

        double maxValue = maxIt->foundParameters.at(id);
        double minValue = minIt->foundParameters.at(id);
        const auto& trueParameter = output.inputData.trueParameters.at(id);
        maxValue = std::abs(trueParameter - maxValue);
        minValue = std::abs(trueParameter - minValue);

        return std::max(maxValue, minValue);
    }

    int MonteCarloEngine::calculateDegreesOfFreedom(const ParameterMap& trueParameters, const ParameterMap& fixedValues)
    {
        return static_cast<int>(trueParameters.size() - fixedValues.size());
    }

    double MonteCarloEngine::getUncertaintyMultiplier(uint8_t numberOfParameters, ConfidenceLevel level)
    {
        return m_uncertaintyMultipliers.at(level).at(numberOfParameters);
    }

    double MonteCarloEngine::calculateMaximumError(const PlotData& trueData, double noiseFactor)
    {
        std::vector<double> min{ trueData.currentData.begin(), trueData.currentData.end() };
        std::vector<double> max{ trueData.currentData.begin(), trueData.currentData.end() };
        double minFactor{ 1 - noiseFactor }, maxFactor{ 1 + noiseFactor };
        std::ranges::for_each(min, [&](double& x) { x *= minFactor; });
        std::ranges::for_each(max, [&](double& x) { x *= maxFactor; });
        Chi2ErrorModel errorModel;
        double error = errorModel.CalculateError({ min.begin(), min.end() }, { max.begin(), max.end() });
        return error;
    }

} // namespace JFMService
