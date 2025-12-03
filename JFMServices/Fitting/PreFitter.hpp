#pragma once
#include "pch.hpp"
#include "JFMIFitting.hpp"

namespace JFMService
{

	class AbstractPreFit
	{
	public:
		AbstractPreFit();
        virtual ~AbstractPreFit() = default;
		std::pair<size_t, size_t> RangeData(const FittingService::PlotData& characteristic);
		virtual FittingService::ParameterMap Estimate(const FittingService::EstimateInput& input) = 0;

	protected:
		std::vector<std::pair<double,double>> m_AMultiplier;
		double adjustCoefficient(double dV);
	private:
		size_t getLowerRange(const FittingService::PlotData& characteristic);
		size_t getUpperRange(const FittingService::PlotData& characteristic);
		
		std::vector<double> dV = { 2.000000e-01, 5.900000e-01,
			6.400000e-01,
			3.400000e-01,
			3.500000e-01,
			3.800000e-01,
			6.300000e-01,
			3.100000e-01,
			5.700000e-01,
			5.300000e-01,
			6.200000e-01,
			5.800000e-01,
			6.100000e-01,
			3.300000e-01,
			6.000000e-01,
			5.600000e-01,
			5.500000e-01,
			5.400000e-01,
			5.200000e-01,
			5.100000e-01,
			2.800000e-01,
			5.000000e-01,
			1.100000e-01,
			4.900000e-01,
			4.800000e-01,
			4.700000e-01,
			4.600000e-01,
			4.500000e-01,
			3.000000e-01,
			4.400000e-01,
			4.300000e-01,
			2.600000e-01,
			4.200000e-01,
			2.400000e-01,
			4.100000e-01,
			4.000000e-01,
			3.900000e-01,
			3.700000e-01,
			3.600000e-01,
			3.200000e-01,
			2.900000e-01,
			1.800000e-01,
			2.700000e-01,
			1.400000e-01,
			2.500000e-01,
			2.300000e-01,
			1.500000e-01,
			2.200000e-01,
			1.300000e-01,
			2.100000e-01,
			1.700000e-01,
			1.900000e-01,
			1.600000e-01,
			1.200000e-01 };
		std::vector<double> alpha = {
	9.938977e-01,
	9.999910e-01,
	9.999961e-01,
	9.994633e-01,
	9.995530e-01,
	9.997186e-01,
	9.999954e-01,
	9.991203e-01,
	9.999874e-01,
	9.999796e-01,
	9.999945e-01,
	9.999894e-01,
	9.999935e-01,
	9.993629e-01,
	9.999924e-01,
	9.999850e-01,
	9.999819e-01,
	9.999788e-01,
	9.999880e-01,
	9.999636e-01,
	9.985519e-01,
	9.999912e-01,
	9.510144e-01,
	9.999499e-01,
	9.999937e-01,
	9.999308e-01,
	9.999956e-01,
	9.999037e-01,
	9.989739e-01,
	9.999969e-01,
	9.998682e-01,
	9.979687e-01,
	9.999978e-01,
	9.971491e-01,
	9.998163e-01,
	9.999979e-01,
	9.997601e-01,
	9.996714e-01,
	9.996133e-01,
	9.992590e-01,
	9.987812e-01,
	9.909561e-01,
	9.982975e-01,
	9.766758e-01,
	9.976616e-01,
	9.965657e-01,
	9.819534e-01,
	9.958300e-01,
	9.711302e-01,
	9.949539e-01,
	9.880638e-01,
	9.926324e-01,
	9.850170e-01,
	9.588665e-01 };
	
};

	class FourParameterModelPreFit :public AbstractPreFit
	{
	public:
		FourParameterModelPreFit();
		virtual FittingService::ParameterMap Estimate(const FittingService::EstimateInput& input) override;
		std::pair<size_t, size_t> rangeData(const FittingService::PlotData& characteristic) { return this->RangeData(characteristic); };

	};

	class SixParameterModelPreFit :public AbstractPreFit
	{
	public:
		SixParameterModelPreFit();
		virtual FittingService::ParameterMap Estimate(const FittingService::EstimateInput& input) override;
		std::pair<size_t, size_t> rangeData(const FittingService::PlotData& characteristic) { return this->RangeData(characteristic); };
	};
	class FiveParameterModelPreFit :public AbstractPreFit
	{
	public:
		FiveParameterModelPreFit();
		virtual FittingService::ParameterMap Estimate(const FittingService::EstimateInput& input) override;
		std::pair<size_t, size_t> rangeData(const FittingService::PlotData& characteristic) { return this->RangeData(characteristic); };
	};
	class SevenParameterModelPreFit :public AbstractPreFit
	{
	public:
		SevenParameterModelPreFit();
		virtual FittingService::ParameterMap Estimate(const FittingService::EstimateInput& input) override;
		std::pair<size_t, size_t> rangeData(const FittingService::PlotData& characteristic) { return this->RangeData(characteristic); };
	};

	class PreFitter
	{
		using PreFitterMap = std::unordered_map<FittingService::ModelID, std::shared_ptr<AbstractPreFit>>;

	public:
		PreFitter();
		std::pair<size_t, size_t> RangeData(const FittingService::PlotData& characteristic);
		FittingService::ParameterMap Estimate(const FittingService::EstimateInput& input);
		std::shared_ptr<AbstractPreFit> operator[](const FittingService::ModelID id);
	private:
		PreFitterMap preFitterMap{};
	};
}
