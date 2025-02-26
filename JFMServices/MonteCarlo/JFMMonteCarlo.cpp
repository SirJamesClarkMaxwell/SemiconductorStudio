#include "JFMMonteCarlo.hpp"

void JFMService::MonteCarlo::Simulate(const MCInput &input, std::function<void(MCOutput &&)> callback)
{
    engine.Simulate(input, callback);
}

double JFMService::MonteCarlo::GetUncertainty(const MCOutput& output, int level, ParameterID id)
{
    return engine.GetUncertainty(output, level, id);
}
