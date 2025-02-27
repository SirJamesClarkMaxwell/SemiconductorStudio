#pragma once
#include "../Fitting/JFMIFitting.hpp"
#include "JFMAdditionalParameters.hpp"

namespace JFMService
{



	using namespace FittingService;
	class DataCalculator
	{
	public:
		DataCalculator() = default;
		void CalculateData(CalculatingData &input);
		double CalculateError(const std::span<double> &original, const std::span<double> &checked);
	};
}