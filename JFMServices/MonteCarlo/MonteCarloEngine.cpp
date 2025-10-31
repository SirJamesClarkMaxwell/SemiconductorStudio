#include "MonteCarloEngine.hpp"
#include "../Models/JFMErrorModel.hpp"
#include "../Fitting/JFMFitter.hpp"
#include "../Models/CalculateData.hpp"
#include <utils.hpp>
#include <compare>
#include <thread>
extern std::vector<std::pair<std::vector<double>, std::vector<double>>> globalNoisyI;
std::mutex g_mutex;
static int blockNumber = 0;
namespace JFMService
{
	MonteCarloEngine::MonteCarloEngine()
		: m_iterationCount(0)
		, m_blockNumber(0)
	{
	}

	void MonteCarloEngine::simulate(
		const std::shared_ptr<AbstractPreFit> preFitter,
		const std::shared_ptr<Fitters::AbstractFitter> fitter,
		MCInput &input,
		MCResult& result)
	{
		MCInput copied{ input };
		auto characteristic = input.startingData.initialData.characteristic;
		std::vector<double> current{ characteristic.currentData.begin(), characteristic.currentData.end() };
		copied.startingData.initialData.characteristic.currentData = { current.begin(), current.end() };

		auto outOfBounds = [](const ParameterMap& PMap, const ParamBounds& bounds)
			{
				for (const auto& [key, val] : PMap)
				{
					if (val < bounds.at(key).first or val > bounds.at(key).second)
						return true;
				}
				return false;
		};

		do {
			current = { characteristic.currentData.begin(), characteristic.currentData.end() };
			copied.startingData.initialData.characteristic.currentData = { current.begin(), current.end() };
			for (auto& I : current)
				generateNoise(I, copied.noise);

			copied.startingData.initialValues = preFitter->Estimate(copied.startingData.initialData);

			fitter->Fit(copied.startingData,
							[&](const ParameterMap&& fittingResult) {
								result.foundParameters = fittingResult;
							});

			calculateFittingError(input, result);
		} while ((result.error > 23.5) or outOfBounds(result.foundParameters, input.startingData.bounds));

		m_iterationCount++;
	}

	void MonteCarloEngine::SimulateImpl(const MCInput& input, std::function<void(MCOutput&&)> callback)
	{
		MCOutput output;
		output.inputData = input;
		std::shared_ptr<Fitters::AbstractFitter> fitter = m_fitter[input.startingData.initialData.modelID];
		std::shared_ptr<AbstractPreFit> preFitter = m_prefitter[input.startingData.initialData.modelID];
		JFM_ASSERT(input.iterations);
		output.mcResult.resize(input.iterations);

#	if defined(JFM_ITER_SIMULATE)
		for (size_t iteration = 0; iteration < output.inputData.iterations; ++iteration)
		MEASURE_TIME("simulate",
			simulate(preFitter, fitter, output.inputData, output.mcResult[iteration]);
		);
#	else
		auto idealParameters = input.trueParameters;
		std::vector<ParameterMap> parameters;
		int steps_per_param = input.iterations;

		// Calculate step sizes based on range
		ParameterMap currentParameters = idealParameters;
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

		auto cartesian = std::views::cartesian_product(pSets[0], pSets[1], pSets[2], pSets[3]);
		output.mcResult.resize(cartesian.size());

		size_t i = 0;
		for (size_t productIdx = 0; productIdx < cartesian.size(); ++productIdx)
		{
			auto&&t = cartesian[productIdx];
			MCResult& result = output.mcResult[productIdx];

			ParameterMap& param = result.foundParameters;
			param[0] = std::get<0>(t);
			param[1] = std::get<1>(t);
			param[2] = std::get<2>(t);
			param[3] = std::get<3>(t);

			calculateFittingError(input, result);
		}
#endif
		if (callback)
			callback(std::move(output));
	}

	void MonteCarloEngine::generateNoise(double& value, double factor)
	{
		double noise = (value * factor / 100);
		double copy = value;
		std::normal_distribution<double> distribution{ 0, 1 };
		value += distribution(generator) * noise;
	}

	void MonteCarloEngine::calculateFittingError(const MCInput& input, MCResult& result)
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

		std::vector<double> trueCurrentVector{};
		trueCurrentVector.resize(current.size());
		CalculatingData trueData = data;
		trueData.parameters = input.trueParameters;
		trueData.characteristic.currentData = std::span<double>{ trueCurrentVector };
		calculator.CalculateData(trueData);

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
