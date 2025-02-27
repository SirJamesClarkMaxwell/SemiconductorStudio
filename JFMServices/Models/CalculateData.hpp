#pragma once
#include "../Fitting/JFMIFitting.hpp"
#include "JFMAdditionalParameters.hpp"

namespace JFMService
{

	enum JFMCharacteristicType
	{
		Dark = 0,
		Light
	};
	enum JFMModelID
	{
		None = 0,
		Model4P = 3,
		Model4PLight = 4,
		Model6P = 5,
		Model6PLight = 6,
	};
	using namespace FittingService;
	class DataCalculator
	{
	public:
		DataCalculator() = default;
		void CalculateData(CalculatingData &input);
		double CalculateError(const std::span<double> &original, const std::span<double> &checked);
	};
}