#include "JFMAdditionalParameters.hpp"

namespace JFMService
{
JFMAdditionalParameters::JFMAdditionalParameters()
    : Temperature(-1.0)
    , fixingConfiguration(static_cast<FixingConfiguration>(0))
{
}

JFMAdditionalParameters::JFMAdditionalParameters(
    double Temperature)
    : Temperature(Temperature)
    , fixingConfiguration(static_cast<FixingConfiguration>(0))
{
}

JFMAdditionalParameters::JFMAdditionalParameters(
    double temperature,
    JFMParameters fixingValues,
    FixingConfiguration configuration)
    : Temperature(temperature)
    , fixingValues(fixingValues)
    , fixingConfiguration(configuration)
{
}
}
