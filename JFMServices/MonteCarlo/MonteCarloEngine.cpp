#include "MonteCarloEngine.hpp"
#include "../Models/JFMErrorModel.hpp"
#include "../Fitting/JFMFitter.hpp"
#include "../Models/CalculateData.hpp"
#include <utils.hpp>
#include <compare>
#include <assert.h>
#include <thread>

namespace JFMService
{
	MonteCarloEngine::MonteCarloEngine()
        : m_iterationCount(0)
        , m_blockNumber(0)
    {
    }

	void MonteCarloEngine::simulateChunk(int startIdx, int chunkSize, const std::shared_ptr<AbstractPreFit>& preFitter,
		const std::shared_ptr<Fitters::AbstractFitter> fitter, MCInput& input,
		std::vector<MCResult>& localResults, int numBlock)
	{
		m_blockNumber++;

		for (int i = 0; i < chunkSize; ++i)
		{
			if ((startIdx+i) >= input.iterations)
			{
				localResults.resize(i);
				break;
			}

			simulate(preFitter, fitter, input, localResults, i);
		}
	}

	void MonteCarloEngine::Simulate(const MCInput& input, std::function<void(MCOutput&&)> callback)
	{
		int chunkSize = input.iterations / 2;
		std::jthread workerThread {
			[=]()
			{
				MCOutput output;
				output.inputData = input;
				std::shared_ptr<Fitters::AbstractFitter> fitter = m_fitter[input.startingData.initialData.modelID];
				std::shared_ptr<AbstractPreFit> preFitter = m_prefitter[input.startingData.initialData.modelID];
				auto start = std::chrono::high_resolution_clock().now();
				output.mcResult.resize(input.iterations);


				std::vector<MCResult> finalResults(input.iterations);
#ifdef JFM_MULTITHREADED
				std::vector<std::future<std::vector<MCResult>>> futures;
				int numChunks = (input.iterations + chunkSize - 1) / chunkSize;
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
#else
				for (int i=0;i<output.inputData.iterations;i++)
				{
                MEASURE_TIME("simulate",
					simulate(preFitter, fitter, output.inputData, finalResults, i);
                );
					output.mcResult = finalResults;
				}
#endif
				auto end = std::chrono::high_resolution_clock().now();
				auto miliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
 				assert(input.iterations);
				auto perIteration = miliseconds / input.iterations;
				auto seconds = miliseconds / 1000;
				Info() << "time: " << seconds << " s "
						  << perIteration << " ms per fit" << std::endl;

				if (callback)
					callback(std::move(output));
			} };

		workerThread.detach();
	}

	void MonteCarloEngine::generateNoise(double& value, double factor)
	{
		double noise = (value * factor / 100);
		double copy = value;
		std::normal_distribution<double> distribution{ 0, 1 };
		value += distribution(generator) * noise;
	}

	void MonteCarloEngine::simulate(const std::shared_ptr<AbstractPreFit>& preFitter, const std::shared_ptr<Fitters::AbstractFitter> fitter, MCInput& input, std::vector<MCResult>& results, int i)
	{
		MCResult result;
		MCInput copied{ input };
		auto characteristic = input.startingData.initialData.characteristic;
		std::vector<double> copiedCurrent{ characteristic.currentData.begin(), characteristic.currentData.end() };
		std::vector<double> copiedVoltage{ characteristic.voltageData.begin(), characteristic.voltageData.end() };
		copied.startingData.initialData.characteristic.currentData = { copiedCurrent.begin(), copiedCurrent.end() };
		auto checkParams = [](const ParameterMap& PMap)
			{
				return std::any_of(PMap.begin(), PMap.end(), [](const std::pair<const int, double>& pair)
					{ return pair.second < 0; });
			};
		auto outOfBounds = [](const ParameterMap& PMap, const ParamBounds& bounds)
			{
				for (const auto& [key, val] : PMap)
				{
					if (val < bounds.at(key).first or val > bounds.at(key).second)
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
        MEASURE_TIME("fit",
			fitter->Fit(copied.startingData, callback);
        );
			calculateFittingError(input, result, calculated);
		} while (result.error > 23.5 or outOfBounds(result.foundParameters, input.startingData.bounds));

		results[i] = result;
		m_iterationCount++;
		Info() << "iteration: " << i << std::endl;
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

		std::vector<double> trueCurrentVector{};
		trueCurrentVector.resize(copiedCurrent.size());
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
		for (const auto& [trueI, fitI] : std::views::zip(trueData.characteristic.currentData, fittedCurrent))
				accumulatedError += IerrorModel(trueI, fitI);

		result.error = (accumulatedError ) ;
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

	/*
	MonteCarloEngine::UncertaintyMultipliers MonteCarloEngine::initializeUncertaintyMultipliers()
	{
	m_uncertaintyMultipliers = {
			{1.0, 4.0, 9.0},
			{2.30, 6.18, 11.8},
			{3.53, 8.02, 14.2},
			{4.72, 9.72, 16.3},
			{5.89, 11.3, 18.2},
			{7.04, 12.8, 20.1}};
	}
	*/
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
