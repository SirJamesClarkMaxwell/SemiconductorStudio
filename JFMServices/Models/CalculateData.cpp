#include "CalculateData.hpp"
#include "JFMModels.hpp"
#include "JFMErrorModel.hpp"
void JFMService::DataCalculator::CalculateData(CalculatingData &input)
{
    FourParameterModel model4;
    SixParameterModel model6;
    FiveParameterModel model5;
    SevenParameterModel model7;
    
    switch (input.modelID)
    {
    case Model4P:
        model4.call(input);
        break;
    case Model6P:
        model6.call(input);
        break;
    case Model4PLight:
        model5.call(input);
        break;
    case Model6PLight:
        model7.call(input);
        break;
    default:
        break;
    }
}

double JFMService::DataCalculator::CalculateError(const std::span<double> &original, const std::span<double> &checked)
{
    Chi2ErrorModel errorModel;
    return errorModel.CalculateError(original, checked);
}
