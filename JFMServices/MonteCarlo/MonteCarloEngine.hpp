#pragma once
#include "../pch.hpp"
#include "../Fitting/JFMIFitting.hpp"
#include "../Fitting/JFMFitter.hpp"
#include "../Fitting/PreFitter.hpp"
#include <atomic>

namespace JFMService
{
    using namespace FittingService;
    enum ConfidenceLevel : uint8_t
    {
        oneSigma = 0,
        twoSigma,
        threeSigma
    };
    class MonteCarloEngine
    {
    public:
        MonteCarloEngine();
        void Simulate(const MCInput &input, std::function<void(MCOutput &&)> callback);
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

    private:
        int calculateDegreesOfFreedom(const ParameterMap &trueParameters, const ParameterMap &fixedValues);
        double getUncertaintyMultiplier(uint8_t numberOfParameters, ConfidenceLevel level);

        double calculateMaximumError(const PlotData &trueData, double noiseFactor);
        void generateNoise(double &value, double factor);
        void simulate(const std::shared_ptr<AbstractPreFit> &preFitter, const std::shared_ptr<Fitters::AbstractFitter> fitter, MCInput &input, std::vector<MCResult> &results, int i);
        void calculateFittingError(const MCInput &input, MCResult &result, std::vector<double> &calculated);
        void simulateChunk(int startIdx, uint32_t chunkSize, const std::shared_ptr<AbstractPreFit> &preFitter,
                           const std::shared_ptr<Fitters::AbstractFitter> fitter, MCInput &input,
                           std::vector<MCResult> &localResults, int numBlock);

        std::atomic<uint32_t> m_iterationCount;
        std::atomic<uint32_t> m_blockNumber;
    };
}
