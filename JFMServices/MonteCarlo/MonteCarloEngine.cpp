#include "MonteCarloEngine.hpp"
#include "../Models/JFMErrorModel.hpp"
#include "../Fitting/JFMFitter.hpp"
#include "../Models/CalculateData.hpp"
#include <compare>
 #define MULTITHREAD
extern std::vector<std::pair<std::vector<double>, std::vector<double>>> globalNoisyI;
std::mutex g_mutex;
static int blockNumber = 0;
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
			std::cout << "block:" << localNumeber << " idx: " << i << std::endl;
		}
	};

	static std::mutex mutex;
	static int num = 0;
	void MonteCarloEngine::Simulate(const MCInput& input, std::function<void(MCOutput&&)> callback)
	{
		int chunkSize = (input.iterations / 23) + 1; // 41
		std::jthread thread{
			[=]()
			{
				MCOutput output;
				output.inputData = input;
				std::shared_ptr<Fitters::AbstractFitter> fitter = m_fitter[input.startingData.initialData.modelID];
				std::shared_ptr<AbstractPreFit> preFitter = m_prefitter[input.startingData.initialData.modelID];
				auto start = std::chrono::high_resolution_clock().now();
				output.mcResult.resize(input.iterations);

				std::vector<std::future<std::vector<MCResult>>> futures;
				int numChunks = (input.iterations + chunkSize - 1) / chunkSize; // 25

				std::vector<MCResult> finalResults(input.iterations);
#ifdef MULTITHREAD
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
			fitter->Fit(copied.startingData, callback);
			calculateFittingError(input, result, calculated);
		} while (result.error > 23.5 or outOfBounds(result.foundParameters, input.startingData.bounds));
		//} while (0 and (result.error > 23.5 or outOfBounds(result.foundParameters, input.startingData.bounds))); // and any of the parameters is negative
		g_mutex.lock();

		globalNoisyI.push_back({ copiedCurrent, calculated });
		g_mutex.unlock();
		results[i] = result;
		num += 1;
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
		//int dataSize = trueCurrent.size();
		
		// auto calculateSigma = [&](const std::vector<double>& noisedI, const std::span<double>& trueI)
		// {
		// 	double sigma = 0.0;
		// 	for (const auto&[tI,nI]:std::views::zip(trueI,noisedI))
		// 		sigma += std::pow(std::abs((std::log(tI) - std::log(nI))),2)/ dataSize;
		// 	return sigma;
			
		// };

		// double sigma = calculateSigma(copiedCurrent,trueData.characteristic.currentData);

		// double firstError = input.firstFitError;
		auto IerrorModel = [&](double trueI, double fittedI)
			{
			/*
				std::cout << "log(I1), log(I2): " << std::log(fittedI) << " " << std::log(trueI) << std::endl;
				std::cout << "log(I1) - log(2): " << std::log(fittedI) - std::log(trueI) << std::endl;
				std::cout << "(log(I1) - log(2))/sqrt(sigma): " << (std::log(fittedI) - std::log(trueI))/std::sqrt(sigma) << std::endl;
				std::cout << "full; " << std::pow(((std::log(fittedI) - std::log(trueI)) / std::sqrt(sigma)), 2) << std::endl;
			*/
				return std::pow(((std::log(fittedI) - std::log(trueI)) / (/*std::log(trueI) * */noise)), 2);
			};
		for (const auto& [trueI, fitI] : std::views::zip(trueData.characteristic.currentData, fittedCurrent))
			//+for (const auto& [trueI, fitI] : std::views::zip(trueData.characteristic.currentData, fittedCurrent))
				accumulatedError += IerrorModel(trueI, fitI);///dataSize;// - firstError;

		result.error = (accumulatedError ) ;
	}

	std::pair<double, double> MonteCarloEngine::GetUncertainty(const MCOutput& output, int level, ParameterID id)
	{
		int degreesOfFreedom = calculateDegreesOfFreedom(output.inputData.trueParameters, output.inputData.startingData.fixConfig);
		double acceptationLevel = m_uncertaintyMultipliers[degreesOfFreedom - 1][level];//getAcceptationLevel(level, degreesOfFreedom);

		std::vector<MCResult> resultsCpy{ output.mcResult };
		std::vector<MCResult> internalResult;

		auto isAcceptedPoint = [&](const MCResult& res)
			{
				return res.error < acceptationLevel;
			};
		for (MCResult item : resultsCpy | std::views::filter(isAcceptedPoint))
			internalResult.push_back(item);

		auto MCResultComparator = [&](const MCResult& lhs, const MCResult& rhs, auto compObjec)
			{
				return compObjec(lhs.foundParameters.at(id), rhs.foundParameters.at(id));
			};
		auto maxIt = std::max_element(internalResult.begin(), internalResult.end(),
			[&](const auto& lhs, const auto& rhs) {
				return lhs.foundParameters.at(id) < rhs.foundParameters.at(id);
			});

		auto minIt = std::min_element(internalResult.begin(), internalResult.end(),
			[&](const auto& lhs, const auto& rhs) {
				return lhs.foundParameters.at(id) < rhs.foundParameters.at(id);
			});

		// Extract the max and min values
		if(internalResult.size())
		{
			double maxValue = maxIt->foundParameters.at(id);
			double minValue = minIt->foundParameters.at(id);
			return { minValue, maxValue };
		}
		return{ -1,-1 };

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

}