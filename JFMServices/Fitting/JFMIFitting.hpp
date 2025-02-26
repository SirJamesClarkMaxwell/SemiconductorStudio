#pragma once
#include "pch.hpp"
namespace JFMService::FittingService
{
	// NOTE Parameters
	using ParameterID = unsigned int;
	using ParameterName = std::string;
	using Parameters = std::unordered_map<ParameterID, ParameterName>;
	using ParameterMap = std::unordered_map<ParameterID, double>;
	//-----

	// NOTE ModelAdditionalParameters
	using AdditionalParameterID = unsigned int;
	using AdditionalParameterName = std::string;
	using AdditionalParameters = std::unordered_map<AdditionalParameterID, AdditionalParameterName>;
	using AdditionalParameterMap = std::unordered_map<AdditionalParameterID, double>;

	// NOTE Model
	using ModelID = unsigned int;
	using ModelName = std::string;
	using Models = std::unordered_map<ModelID, ModelName>;
	using ModelParameters = std::unordered_map<ModelID, std::vector<ParameterID>>;
	using ModelAdditionalParameters = std::unordered_map<ModelID, std::vector<AdditionalParameterID>>;

	// theoretical bounds for parameters
	using Bounds = std::pair<double, double>;
	using ParamBounds = std::unordered_map<ParameterID, Bounds>;

	struct NumericsConfig
	{
		// model list
		//  ID -- Name
		//--parameter list
		//  ID -- Name
		Models models;
		Parameters parameters;
		ModelParameters modelParameters;
		ParamBounds paramBounds; // theoretical bounds, the most general
	};

	struct PlotData
	{
		std::span<double> voltageData{};
		std::span<double> currentData{};
	};
	struct CalculatingData
	{
		PlotData characteristic{};
		ParameterMap parameters{};
		AdditionalParameterMap additionalParameters{};
		ModelID modelID{};
	};
	struct EstimateInput
	{
		PlotData characteristic{};
		AdditionalParameterMap additionalParameters{};
		ModelID modelID{};
	};
	struct FittingInput
	{
		EstimateInput initialData{};
		std::string_view name{};
		ParameterMap fixConfig{};
		ParameterMap initialValues{};
		ParamBounds bounds{};
		bool useBounds{}; // generation of the simplex from the UI
	};

	struct MCInput
	{
		FittingInput startingData{};
		std::filesystem::path relPath{};
		ParameterMap trueParameters{};
		size_t iterations{};
		double noise{1.0};
		std::pair<double, double> characteristicBounds;
		double firstFitError;
	};

	struct MCResult
	{
		ParameterMap foundParameters{};
		double error{-1.0};
	};

	struct MCOutput
	{
		MCInput inputData{};
		std::vector<MCResult> mcResult{};
	};
	struct MCSave
	{
		std::vector<MCResult> results;
		std::string title;
		std::filesystem::path pathToSave;
		ParameterID x_label;
		ParameterID y_label;
		int degreesOfFreedom;

	};
	struct UncertaintySave {
		ParameterMap paramPair{};
		std::vector<ParameterMap> uncertainty{};
		double T{};
		std::string name{};
	};

	class IFitting
	{
	public:
		virtual NumericsConfig GetConfiguration() = 0;			// note this will return all possible available models
		virtual void CalculateData(CalculatingData &input) = 0; // todo move this into a separate service in SS
		virtual double CalculateError(const std::span<double> &original, const std::span<double> &checked) = 0;

		virtual std::pair<size_t, size_t> RangeData(const PlotData &characteristic) = 0;		  // todo move this into a separate service in SS
		virtual std::unordered_map<ParameterID, double> Estimate(const EstimateInput &input) = 0; // todo add automation as a service
		virtual void Fit(const FittingInput &input, std::function<void(ParameterMap &&)> callback) = 0;

		virtual void Simulate(const MCInput &input, std::function<void(MCOutput &&)> callback) = 0;
		virtual double GetUncertainty(const MCOutput &output, int level, ParameterID id) = 0;
		virtual void SaveMCPlot(const MCSave &toSave) = 0;
		virtual void SaveUncertanties(const std::vector< UncertaintySave>& toSave,const std::filesystem::path& path)=0;
	};
}