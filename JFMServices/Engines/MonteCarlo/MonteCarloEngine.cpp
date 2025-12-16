#include "MonteCarloEngine.hpp"
#include "Models/JFMErrorModel.hpp"
#include "Fitting/JFMFitter.hpp"
#include "Models/CalculateData.hpp"
#include <utils.hpp>
#include <compare>
#include "Calculator/Calculator.h"
#include "JunctionFitMaster.hpp"

namespace JFMService
{
	MonteCarloEngine::MonteCarloEngine()
		: m_iterationCount(0)
		, m_blockNumber(0)
	{
	}

    template <CalculationModeId calculationModeId>
    constexpr ParamsPtr<calculationModeId> MonteCarloEngine::createParams(
        const MCInput &input,
        std::vector<MCResult> *outputs)
    {
        using namespace Calculator;

        ParamsPtr<calculationModeId> params;

        if constexpr (calculationModeId == CalculationModeId::CalculateSimulate)
        {
            JFM_ASSERT(input.iterations > 0);
            outputs->resize(input.iterations);

            params = std::make_unique<CalcParams<calculationModeId>> (
                                        std::move(input),
                                        std::move(outputs),
                                        this,
                                        m_fitter[input.startingData.initialData.modelID],
                                        m_prefitter[input.startingData.initialData.modelID]);
        }
        else if constexpr (calculationModeId == CalculationModeId::CalculateSingleCore ||
                    calculationModeId == CalculationModeId::CalculateMultiCore ||
                    calculationModeId == CalculationModeId::CalculateGpu)
        {
		    auto idealParameters = input.trueParameters;
		    std::vector<ParameterMap> parameters;
		    int steps_per_param = input.iterations;

		    // Calculate step sizes based on range
		    std::vector<std::vector<double>> pSets(idealParameters.size()); // Pre-size the vector
		    for (size_t i = 0; i < idealParameters.size(); ++i)
		    {
		    	double start = input.trueParameters.at(i) * 0.90;
		    	double end = input.trueParameters.at(i) * 1.1;
		    	double stepSize = (end - start) / steps_per_param;
		    	double curr = start;

		    	while (curr <= end) {
		    		pSets[i].push_back(curr);
		    		curr += stepSize;
		    	}
		    }

            params = std::make_unique<CalcParams<calculationModeId>> (
                                        std::move(input),
                                        std::move(outputs),
                                        this,
                                        pSets );
        }
        else
        {
            Unreachable();
        }

        return std::move(params);
    }

    template <CalculationModeId calculationModeId>
    constexpr inline void calculatorCall(ParamsPtr<calculationModeId> params)
    {
        Calculator::CalculatorAll< CalcParams<calculationModeId> >::call( std::ref(*params.get()) );

        // Numeric using cartesian product currently accept each returned value even though
        // it might not having physical backing.
        // We need to remove then from the output array
        if constexpr (calculationModeId == CalculationModeId::CalculateSingleCore ||
                      calculationModeId == CalculationModeId::CalculateMultiCore ||
                      calculationModeId == CalculationModeId::CalculateGpu)
        {
            size_t validErrCount = 0;
            auto *output = params->output;

            for (size_t ndx = 0; ndx < output->size(); ++ndx)
            {
                auto e = output->at(ndx).error;

                if (e < static_cast<decltype(e)>(23.5))
                {
                    output->at(validErrCount).error = e;
                    output->at(validErrCount).foundParameters = output->at(ndx).foundParameters;
                    validErrCount++;
                }
            }
            output->resize(validErrCount);
        }
    }

	void MonteCarloEngine::SimulateImpl(
        const MCInput& input,
        std::function<void(MCOutput&&)> callback)
	{
		MCOutput output;
		output.inputData = input;
		auto &outputs = output.mcResult;
        using namespace Calculator;

        auto params = createParams<JunctionFitMaster::modeId>(input, &outputs);
        calculatorCall<JunctionFitMaster::modeId>( std::move(params) );

		if (callback)
			callback(std::move(output));
	}

	double MonteCarloEngine::generateNoise(double value, double factor)
	{
		double noise = (value * factor / 100);
		std::normal_distribution<double> distribution{ 0, 1 };
		return distribution(generator) * noise;
	}

	void MonteCarloEngine::simulateSingleIteration(
        const std::shared_ptr<AbstractPreFit>& preFitter,
        const std::shared_ptr<Fitters::AbstractFitter> fitter,
        MCInput& input,
        MCResult &result)
	{
		MCInput copied{ input };
		auto characteristic = input.startingData.initialData.characteristic;
		std::vector<double> current{ characteristic.currentData.begin(), characteristic.currentData.end() };
		copied.startingData.initialData.characteristic.currentData = { current.begin(), current.end() };

		static auto outOfBounds = [](const ParameterMap& PMap, const ParamBounds& bounds)
		{
			for (const auto& [key, val] : PMap)
			{
				if ((val < bounds.at(key).first) || (val > bounds.at(key).second))
					return true;
			}
			return false;
		};

		do
		{
			current = { characteristic.currentData.begin(), characteristic.currentData.end() };
			copied.startingData.initialData.characteristic.currentData = { current.begin(), current.end() };
			for (auto& I : current)
            {
				I += generateNoise(I, copied.noise);
            }

			copied.startingData.initialValues = preFitter->Estimate(copied.startingData.initialData);
			fitter->Fit(copied.startingData,
						[&](const ParameterMap&& fittingResult)
						{ result.foundParameters = fittingResult; });

			CalculateError(input, result);
		} while (result.error > 23.5 || outOfBounds(result.foundParameters, input.startingData.bounds));
	}

	void MonteCarloEngine::CalculateError(const MCInput& input, MCResult& result)
	{
		auto characteristic = input.startingData.initialData.characteristic;
		std::vector<double> current{ characteristic.currentData.begin(), characteristic.currentData.end() };
		std::vector<double> voltage{ characteristic.voltageData.begin(), characteristic.voltageData.end() };

		CalculatingData data;
		data.additionalParameters = input.startingData.initialData.additionalParameters;
		data.parameters = result.foundParameters;
		data.characteristic = { {voltage.begin(), voltage.end()}, {current.begin(), current.end()} };
		data.modelID = input.startingData.initialData.modelID;

		DataCalculator calculator;
		calculator.CalculateData(data);
        {
            static bool dumped = false;
            if (dumped == false)
            {
                auto vec = std::vector<double>{data.characteristic.currentData.begin(), data.characteristic.currentData.end()};
                jfm_debug::DataDumper("cpu : 1", vec.data(), vec.size());
                dumped = true;
            }
        }

		std::vector<double> trueCurrentVector{};
		trueCurrentVector.resize(current.size());
		CalculatingData trueData = data;
		trueData.parameters = input.trueParameters;
		trueData.characteristic.currentData = std::span<double>{ trueCurrentVector };
		calculator.CalculateData(trueData);
        {
            static bool dumped = false;
            if (dumped == false)
            {
                auto vec = std::vector<double>{trueData.characteristic.currentData.begin(), trueData.characteristic.currentData.end()};
                jfm_debug::DataDumper("cpu : 2", vec.data(), vec.size());

                auto vecFitted = std::vector<double>{trueData.characteristic.currentData.begin(), trueData.characteristic.currentData.end()};
                jfm_debug::DataDumper("cpu : Fitted", vecFitted.data(), vecFitted.size());
                dumped = true;
            }
        }

		std::span<double> fittedCurrent = data.characteristic.currentData;
		double accumulatedError = 0.0;
		double noise = input.noise / 100.0;

		for (const auto& [trueI, fitI] : std::views::zip(trueData.characteristic.currentData, fittedCurrent))
		{
			accumulatedError += std::pow(((std::log(fitI) - std::log(trueI)) / (noise)), 2);
		}
		result.error = (accumulatedError);
	}

	double MonteCarloEngine::GetUncertainty(const MCOutput& output, int level, ParameterID id)
	{
		int degreesOfFreedom = calculateDegreesOfFreedom(output.inputData.trueParameters, output.inputData.startingData.fixConfig);

		std::vector<MCResult> resultsCpy{ output.mcResult };

		auto MCResultComparator = [&](const MCResult& lhs, const MCResult& rhs, auto compObjec)
			{
				return compObjec(lhs.foundParameters.at(id), rhs.foundParameters.at(id));
			};
		auto maxIt = std::max_element(resultsCpy.begin(), resultsCpy.end(),
			[&](const auto& lhs, const auto& rhs) {
				return lhs.foundParameters.at(id) < rhs.foundParameters.at(id);
			});

		auto minIt = std::min_element(resultsCpy.begin(), resultsCpy.end(),
			[&](const auto& lhs, const auto& rhs) {
				return lhs.foundParameters.at(id) < rhs.foundParameters.at(id);
			});

		// Extract the max and min values
		double maxValue = maxIt->foundParameters.at(id);
		double minValue = minIt->foundParameters.at(id);
		const auto& trueParameter = output.inputData.trueParameters.at(id);
		maxValue = std::abs(trueParameter - maxValue);
		minValue = std::abs(trueParameter - minValue);

		return std::max(maxValue,minValue);
	};

	int MonteCarloEngine::calculateDegreesOfFreedom(const ParameterMap& trueParameters, const ParameterMap& fixedValues)
	{
		return trueParameters.size() - fixedValues.size();
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
		std::ranges::for_each(min, [&](double& x)
			{ return x *= minFactor; });
		std::ranges::for_each(max, [&](double& x)
			{ return x *= maxFactor; });
		Chi2ErrorModel errorModel;
		double error = errorModel.CalculateError({ min.begin(), min.end() }, { max.begin(), max.end() });
		return error;
	}
} // namespace JFMService
