#pragma once

#include "Fitting/JFMIFitting.hpp"
#include "MonteCarloEngine.hpp"

namespace JFMService
{
    class MonteCarlo
    {
    public:
        MonteCarlo();
        ~MonteCarlo();

        void Simulate(const MCInput& input,
                      std::function<void(MCOutput&&)> callback);

        double GetUncertainty(const MCOutput& output,
                              int level,
                              ParameterID id);
    private:
        MonteCarloEngine *engine;
    };
} // namespace JFMService
