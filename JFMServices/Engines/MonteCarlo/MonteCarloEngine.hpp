#pragma once
#include "SimulationEngine.hpp"
#include "Fitting/JFMIFitting.hpp"
#include "Fitting/JFMFitter.hpp"
#include "Fitting/PreFitter.hpp"
#include <atomic>
#include <ranges>
#include <memory>
#include "Calculator/Calculator.h"

using namespace utils;

namespace JFMService
{
	using namespace FittingService;

	enum ConfidenceLevel : uint8_t
	{
		oneSigma = 0,
		twoSigma,
		threeSigma
	};

    template <CalculationModeId>
    struct CalculationParamsAll;

    template<>
    struct CalculationParamsAll<CalculationModeId::CalculateSingleCore>
    {
        using type = Calculator::CalculationParamsCpuSingle;
    };

    template<>
    struct CalculationParamsAll<CalculationModeId::CalculateMultiCore>
    {
        using type = Calculator::CalculationParamsCpuMulti;
    };

    template<>
    struct CalculationParamsAll<CalculationModeId::CalculateSimulate>
    {
        using type = Calculator::CalculationParamsSimulate;
    };

    template<>
    struct CalculationParamsAll<CalculationModeId::CalculateGpu>
    {
        using type = Calculator::CalculationParamsGpu;
    };

    template <CalculationModeId modeId>
    using CalcParams = CalculationParamsAll<modeId>::type;

    template <CalculationModeId modeId>
    using ParamsPtr = std::unique_ptr<CalcParams<modeId>>;

	class MonteCarloEngine : public SimulationEngine
	{
	public:
		MonteCarloEngine();

        void CalculateError(const MCInput& input, MCResult& result) override;

		void Simulate(const MCInput& input, std::function<void(MCOutput&&)> callback) override
		{
			MEASURE_TIME_THIS_FUNC( SimulateImpl(input, callback); );
		}

        void simulateSingleIteration(const std::shared_ptr<AbstractPreFit>& preFitter,
                                     const std::shared_ptr<Fitters::AbstractFitter> fitter,
                                     MCInput& input,
                                     MCResult &result);

		double GetUncertainty(const MCOutput &output, int level, ParameterID id);

	private:
		using UncertaintyMultipliers = std::vector<std::array<double, 3>>;
		const UncertaintyMultipliers m_uncertaintyMultipliers{
			{1.0, 4.0, 9.0},
			{2.30, 6.18, 11.8},
			{3.53, 8.02, 14.2},
			{4.72, 9.72, 16.3},
			{5.89, 11.3, 18.2},
			{7.04, 12.8, 20.1} };

		Fitters::Fitter m_fitter;
		PreFitter m_prefitter;

		inline static thread_local std::mt19937 generator{std::random_device{}()};

		int calculateDegreesOfFreedom(const ParameterMap &trueParameters, const ParameterMap &fixedValues);
		double getUncertaintyMultiplier(uint8_t numberOfParameters, ConfidenceLevel level);

		double calculateMaximumError(const PlotData &trueData, double noiseFactor);
		double generateNoise(double value, double factor);

		void SimulateImpl(const MCInput &input, std::function<void(MCOutput &&)> callback);

        template <CalculationModeId calculationModeId>
        constexpr ParamsPtr<calculationModeId> createParams(
            const MCInput &input,
            std::vector<MCResult> *outputs);

		std::atomic<uint32_t> m_iterationCount;
		std::atomic<uint32_t> m_blockNumber;
	};
}
