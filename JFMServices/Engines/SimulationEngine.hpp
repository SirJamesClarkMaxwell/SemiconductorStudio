#pragma once

#include "JFMIFitting.hpp"

namespace JFMService
{
    using FittingService::MCInput;
    using FittingService::MCResult;
    using FittingService::MCOutput;

    class SimulationEngine
    {
    public:
        virtual ~SimulationEngine()
        {
        }

        virtual void CalculateError(const MCInput& input, MCResult& result) = 0;
        virtual void Simulate(const MCInput& input, std::function<void(MCOutput&&)> callback) = 0;
    };
} // namespace JFMService
