#include "MonteCarloEngine.hpp"
#include "../Models/JFMErrorModel.hpp"
#include "../Fitting/JFMFitter.hpp"
#include "../Models/CalculateData.hpp"
#include <utils.hpp>
#include <compare>
#define MULTITHREAD
extern std::vector<std::pair<std::vector<double>, std::vector<double>>> globalNoisyI;
std::mutex g_mutex;
static int blockNumber = 0;
#include <assert.h>
#include <thread>

namespace JFMService
{
	MonteCarloEngine::MonteCarloEngine() {};
	void MonteCarloEngine::simulateChunk(int startIdx, int chunkSize, const std::shared_ptr<AbstractPreFit>& preFitter,
		const std::shared_ptr<Fitters::AbstractFitter> fitter, MCInput& input,
		std::vector<MCResult>& localResults, int numBlock)
	{
		blockNumber += 1;
		int localNumeber = blockNumber;
		// int blockNumber = startIdx - chunkSize
		for (int i = 0; i < chunkSize; ++i)
		{
			int idx = startIdx + i;
			if (idx >= input.iterations)
			{
				localResults.resize(i);
				break;
			}
			MCResult result;
			simulate(preFitter, fitter, input, localResults, i);
			// if(i%20 == 0)
			//std::cout << "block:" << localNumeber << " idx: " << i << std::endl;
		}
	};

	static std::mutex mutex;
	static int num = 0;
	void MonteCarloEngine::Simulate(const MCInput& input, std::function<void(MCOutput&&)> callback)
	{ 
		int chunkSize = input.iterations / 12; // 41
		std::jthread thread{
			[=]()
			{
				MCOutput output;
				output.inputData = input;
				std::shared_ptr<Fitters::AbstractFitter> fitter = m_fitter[input.startingData.initialData.modelID];
				std::shared_ptr<AbstractPreFit> preFitter = m_prefitter[input.startingData.initialData.modelID];
				auto start = std::chrono::high_resolution_clock().now();
				output.mcResult.resize(input.iterations);


				std::vector<MCResult> finalResults(input.iterations);
#ifdef MULTITHREAD
				std::vector<std::future<std::vector<MCResult>>> futures;
				int numChunks = (input.iterations + chunkSize - 1) / chunkSize; // 25
				for (int chunk = 0; chunk < numChunks; ++chunk)
				{
					int startIdx = chunk * chunkSize;
					futures.push_back(std::async(std::launch::async,
												 [&, startIdx]()
												 {
													 std::vector<MCResult> localResults(chunkSize);
													 simulateChunk(startIdx, chunkSize, preFitter, fitter, output.inputData, localResults, chunk);
													 return localResults; // Return local results
												 }));
				}

				for (int chunk = 0; chunk < numChunks; ++chunk)
				{
					auto localResults = futures[chunk].get(); // Wait for and retrieve local results
					std::copy(localResults.begin(), localResults.end(), output.mcResult.begin() + (chunk * chunkSize));
				}
#endif
#ifndef MULTITHREAD
				for (int i=0;i<output.inputData.iterations;i++)
				{
					simulate(preFitter, fitter, output.inputData, finalResults, i);
					output.mcResult = finalResults;
				}
#endif
				auto end = std::chrono::high_resolution_clock().now();
				auto miliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
				auto perIteration = miliseconds / input.iterations;
				auto seconds = miliseconds / 1000;
				std::cout << "time: " << seconds << " s "
						  << perIteration << " ms per fit" << std::endl;

				if (callback)
					callback(std::move(output));
			} };

		thread.detach();
	}

	void MonteCarloEngine::generateNoise(double& value, double factor)
	{
		double noise = (value * factor / 100);
		double copy = value;
		// std::uniform_real_distribution<double> distribution{ -1,1 };
		std::normal_distribution<double> distribution{ 0, 1 };
		// std::cout << distribution(m_generator)*sigma << std::endl;
		//std::cout << value << " ";
		value += distribution(m_generator) * noise;
		//std::cout << value << std::endl;
		// value = value +  distribution(m_generator)*(factor / 100) * value ;
		// value = std::abs(value);
	}
	void MonteCarloEngine::simulate(const std::shared_ptr<AbstractPreFit>& preFitter, const std::shared_ptr<Fitters::AbstractFitter> fitter, MCInput& input, std::vector<MCResult>& results, int i)
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

	void MonteCarloEngine::Simulate(const MCInput& input, std::function<void(MCOutput&&)> callback)
	{
		MCOutput output;
		output.inputData = input;
		std::shared_ptr<Fitters::AbstractFitter> fitter = m_fitter[input.startingData.initialData.modelID];
		std::shared_ptr<AbstractPreFit> preFitter = m_prefitter[input.startingData.initialData.modelID];
		assert(input.iterations);
		output.mcResult.resize(input.iterations);

		std::vector<MCResult> finalResults(input.iterations);
		output.mcResult.resize(output.inputData.iterations);

		for (int iteration = 0; iteration < output.inputData.iterations; ++iteration) {
			MEASURE_TIME("simulate",
				simulate(preFitter, fitter, output.inputData, output.mcResult[iteration]);
			);
			Info() << "iteration: " << iteration + 1 << std::endl;
		}

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
