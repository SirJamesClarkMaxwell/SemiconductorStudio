#include "JFMFitter.hpp"
#include "../Models/JFMAdditionalParameters.hpp"
#include "../Models/JFMModels.hpp"
#include "../Models/CalculateData.hpp"
#include "FittingSetup.hpp"

#include "../NumericStorm.hpp"
namespace JFMService::Fitters
{
	//! Abstract Fitter
	template <size_t parameter_size>
	IVFittingSetup<parameter_size> transferFittingSetUp(const FittingInput &input)
	{
		IVFittingSetup<parameter_size> setUp;
		ParameterMap initials = input.initialValues;
		std::vector<double> min, max;
		min.reserve(parameter_size);
		max.reserve(parameter_size);
		// Convert unordered_map to a vector for sorting
		std::vector<std::pair<unsigned int, double>> sortedMinBounds,sortedMaxBounds;

		// Fix: Ensure the correct type conversion when inserting into sortedBounds
		for (const auto& [index,item] : input.bounds) 
		{
			sortedMinBounds.emplace_back(index ,item.first);
			sortedMaxBounds.emplace_back(index ,item.second );
		}
		std::sort(sortedMinBounds.begin(), sortedMinBounds.end(), [](const auto& a, const auto& b) {
			return a.first < b.first;
			});
		std::sort(sortedMaxBounds.begin(), sortedMaxBounds.end(), [](const auto& a, const auto& b) {
			return a.first < b.first;
			});

		for (size_t i = 0; i < sortedMinBounds.size() && i < sortedMaxBounds.size(); ++i)
		{
			min.emplace_back(sortedMinBounds[i].second);
			max.emplace_back(sortedMaxBounds[i].second);
		}
		std::array<double, parameter_size> minArray,maxArray;
		std::copy(min.begin(), min.end(), minArray.begin());
		std::copy(max.begin(), max.end(), maxArray.begin());
		setUp.simplexMin = { minArray };
		setUp.simplexMax = {maxArray};

		return setUp;
	}
	template <size_t parameter_size>
	NumericStorm::Fitting::Parameters<parameter_size> transferInitialPoint(const ParameterMap &initial)
	{
		int i = 0;
		NumericStorm::Fitting::Parameters<parameter_size> initialPoint;
		for (auto [index, value] : initial)
		{
			initialPoint[index] = initial.at((ParameterID)index);
			i++;
		}

		return initialPoint;
	}
	template <class Model, size_t parameter_size>
	SimplexOptimizationResults<parameter_size> fit(const IVFittingSetup<parameter_size> &setUp, const NumericStorm::Fitting::Parameters<parameter_size> &initialPoint, const Data &data, const JFMAdditionalParameters &additionalParameters)
	{
		using NSFitter = NumericStorm::Fitting::Fitter<IVSimplexOptimizer<Model>>;
		NSFitter fitter = getFitter<Model, parameter_size>(setUp);
		return fitter.fit(initialPoint, data, additionalParameters);
	}
	Data transferFittingData(const PlotData &input)
	{
		Data singleData(2);
		singleData[0] = std::vector<double>(input.voltageData.begin(), input.voltageData.end());
		singleData[1] = std::vector<double>(input.currentData.begin(), input.currentData.end());

		return singleData;
	}
	JFMAdditionalParameters transferAdditionalParameters(const AdditionalParameterMap &input, const ParameterMap &fixingConfig)
	{
		JFMAdditionalParameters additional;
		additional.Temperature = input.at(Temperature);
		FixingConfiguration fixingConfiguration = static_cast<FixingConfiguration>(0);
		JFMParameters destination;
		destination.getParameters().resize(6);
		for (const auto & src :  fixingConfig)
		{
			auto &[key, val] = src;
			destination[key] = val;
			fixingConfiguration |= (FixingConfiguration)BIT(key);
		}
		additional.fixingConfiguration = fixingConfiguration;
		additional.fixingValues = destination;
		return additional;
	}

	template<class CurrentModel, size_t parameter_size>
	void fit_current(const FittingInput& input, Callback callback)
	{
		int fittingIterationRuns = 0;
		auto checkRepetitionCondition = [&](const SimplexOptimizationResults<parameter_size>& result)
		{
			auto parameters = result.getParameters();
			bool negativeValueParameters = std::ranges::any_of(parameters, [](double value)
			{ return value < 0; });
			bool bigError = result.getError() > 1 || result.getError() < 0;
			bool iterationCondition = fittingIterationRuns < 1;
			return (negativeValueParameters || bigError) && iterationCondition;
		};
	
		IVFittingSetup<parameter_size> setUp = transferFittingSetUp<parameter_size>(input);
		Data NSDdata = transferFittingData(input.initialData.characteristic);
		JFMAdditionalParameters additionalParameters = transferAdditionalParameters(input.initialData.additionalParameters, input.fixConfig);
		NumericStorm::Fitting::Parameters<parameter_size> initialPoint = transferInitialPoint<parameter_size>(input.initialValues);
	
		auto recalculateBounds = [](IVFittingSetup<parameter_size>& setUp, NumericStorm::Fitting::Parameters<parameter_size>& initial)
		{
			auto min = (NumericStorm::Fitting::SimplexPoint<parameter_size>(initial) * 0.1).getParameters().getParameters();
			auto max = (NumericStorm::Fitting::SimplexPoint<parameter_size>(initial) * 10).getParameters().getParameters();
			setUp.simplexMin = min;
			setUp.simplexMax = max;
		};
	
		auto recalculateInitialPoint = [](NumericStorm::Fitting::Parameters<parameter_size>& initial)
		{
			for (auto& item : initial.getParameters())
				item *= Random::Float(0.1, 10);
		};
	
		SimplexOptimizationResults<parameter_size> results;
		auto transferFixingConfiguration = [&](const ParameterMap& additional)
		{
			auto& destination = results.getParameters();
			for (const auto& [key, val] : additional)
				if (additional.at(key))
					destination[(int)key] = val;
		};
	
		do
		{
			if (fittingIterationRuns > 1)
			{
				recalculateBounds(setUp, initialPoint);
				recalculateInitialPoint(initialPoint);
			}
			results = fit<CurrentModel, parameter_size>(setUp, initialPoint, NSDdata, additionalParameters);
			if (additionalParameters.fixingConfiguration)
				transferFixingConfiguration(input.fixConfig);
			fittingIterationRuns += 1;
		} while (checkRepetitionCondition(results));
	
		ParameterMap fittingResult;
		for (const auto& [index, value] : std::views::enumerate(results.getParameters()))
			fittingResult[(Fitters::ParameterID)index] = value;
	
		if (callback)
			callback(std::move(fittingResult));
	}
	
	//! Dark Characteristic Fitters
	void FourParameterFitter::Fit(const FittingInput &input, Callback callback)
	{
		fit_current<FourParameterModel,4>(input,callback);
	}
	void SixParameterFitter::Fit(const FittingInput &input, Callback callback)
	{
		
		fit_current<SixParameterModel,6>(input,callback);
	}

	//! Light Characteristic Fitters
	void FiveParameterFitter::Fit(const FittingInput &input, Callback callback)
	{
		fit_current<FiveParameterModel,5>(input,callback);
	}
	void SevenParameterFitter::Fit(const FittingInput &input, Callback callback)
	{
		fit_current<SevenParameterModel,7>(input,callback);
	}

	//! General Fitter
	Fitter::Fitter()
	{
		fitterMap[Model4P] = std::make_shared<FourParameterFitter>();
		fitterMap[Model6P] = std::make_shared<SixParameterFitter>();
		fitterMap[Model4PLight] = std::make_shared<FiveParameterFitter>();
		fitterMap[Model6PLight] = std::make_shared<SevenParameterFitter>();

	}
	void Fitter::Fit(const FittingInput &input, Callback callback)
	{
		// todo add checking validity of item inside the map
		fitterMap[input.initialData.modelID]->Fit(input, callback);
	}

	std::shared_ptr<AbstractFitter> Fitter::operator[](ModelID id)
	{
		return fitterMap.at(id);
	}
}