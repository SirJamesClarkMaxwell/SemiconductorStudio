#include "PreFitter.hpp"
#include "JFMFitter.hpp"
#include "CalculateData.hpp"

namespace JFMService
{
    AbstractPreFit::AbstractPreFit()
    {
		m_AMultiplier.resize(dV.size());
		for (const auto& [dest, dv, coeff] : std::views::zip(m_AMultiplier, dV, alpha))
			dest = { dv, coeff };
		std::sort(m_AMultiplier.begin(), m_AMultiplier.end(), [&](std::pair<double, double> lhs, std::pair<double, double> rhs)
			{ return lhs.first < rhs.first; });
    }

    std::pair<size_t, size_t> AbstractPreFit::RangeData(const FittingService::PlotData& characteristic)
	{
		return std::make_pair<size_t, size_t>(getLowerRange(characteristic), getUpperRange(characteristic));
	};
    double adjustCoefficient(double dV, const std::vector<std::pair<double,double>>& m_AMultiplier)
    {
		size_t index = std::distance(m_AMultiplier.begin(), std::lower_bound(m_AMultiplier.begin(), m_AMultiplier.end(), dV, [&](std::pair<double, double> lhs, double rhs)
			{ return lhs.first < rhs; }));
		auto interpolate = [&](size_t index)
			{
				if (index == m_AMultiplier.size() - 1)
					index-=1;
				auto& [dvStart, start] = m_AMultiplier[index];
				auto& [dvEnd, end] = m_AMultiplier[index + 1];

				double slope = (end - start) / (dvEnd - dvStart);
				double shift = end - slope * dvEnd;
				return  slope * dV + shift;
			};
		if (dV < m_AMultiplier[0].first)
			return interpolate(0);
		if (dV > m_AMultiplier.back().first)
			return 1.0;

		return interpolate(index);
    }
    size_t AbstractPreFit::getLowerRange(const FittingService::PlotData &characteristic)
    {
		size_t start{ 0 };
		for (const auto& [V, I] : std::views::zip(characteristic.voltageData, characteristic.currentData))
			if (I < 0.0 or V<0.1)
				start++;
		start += 3;
		return start;
	};

	size_t AbstractPreFit::getUpperRange(const FittingService::PlotData& characteristic)
	{

		std::vector<double> copy = { characteristic.currentData.begin(), characteristic.currentData.end() };
		std::reverse(copy.begin(), copy.end());

		auto avg = [&](int first, int window)
			{
				int last = std::min(first + window, static_cast<int>(copy.size()));
				if (first >= last || first < 0)
					return 0.0;
				double sum = std::accumulate(copy.begin() + first, copy.begin() + last, 0.0);
				return sum / (last - first);
			};

		int window = 5;

		for (const auto& [i, item] : std::views::enumerate(copy))
		{
			if ((item - avg(i, window)) < 0.0001)
				continue;
			else
				return copy.size() - i - window;
		}

		return copy.size();
	};

	double estimateRsh(const std::vector<double>& V,const std::vector<double>& I,int&AStart )
	{
		double S{ 0.0 }, S0{ 0.0 };
		size_t n = 2;
		bool sb=false, s0b=false;
		do
		{
			if (n >= V.size() || n >= I.size() || I.size() == 0 || V.size() == 0)
				break;

			// Prevent division by zero
			if ((I[n - 1] - I[n - 2]) == 0 || (I[n - 1] - I[0]) == 0) {
				//Info() << "Skipping iteration due to division by zero\n";
				//continue;
			}

			S = (V[n - 1] - V[n - 2]) / (I[n - 1] - I[n - 2]);
			S0 = (V[n - 1] - V[0]) / (I[n - 1] - I[0]);
			n++;

			sb = std::isinf(S);
			s0b = std::isinf(S0);
			//Info() << "S: " << S << " (valid: " << sb << ")  S0: " << S0 << " (valid: " << s0b << ")\n";
			bool t = S / S0 >= 0.8;
			//Info() << "t: " << t << std::endl;
		} while ((S / S0) >= 0.8 or sb or s0b);

		AStart = n;
		return S0;
	}
	double estimateRs(const std::vector<double>&V,const std::vector<double>&I,const int& AStart,int& AEnd)
	{
		int RsStart = I.size()-1;
		int n = RsStart - 2;
		double S=0,S0=0;
		do
		{
			if (n == 0 || n == AStart)
				break;

			S = (V[n + 2] - V[n + 1]) / (I[n + 2] - I[n + 1]);
			S0 = (V[RsStart] - V[n + 1]) / (I[RsStart] - I[n + 1]);

			n--;
		} while (S / S0 <= 0.8);

		AEnd = n;
		return S0;
	}
	
	struct IdealityFactorAdditionalParameters 
	{
		int AStart, AEnd, maxDerIndex;
		double T,dV,k;
		std::vector<double>logI;

        IdealityFactorAdditionalParameters() = default;

        IdealityFactorAdditionalParameters(int AStart, int AEnd, int maxDerIndex, double T)
            : AStart(AStart)
            , AEnd(AEnd)
            , maxDerIndex(maxDerIndex)
            , T(T)
        {
        }
	};
	double estimateIdealityFactor(const std::vector<double>& V, const std::vector<double>& I,IdealityFactorAdditionalParameters& params) 
	{
		std::vector<double> logI{};

		std::transform(I.begin() + params.AStart, I.begin() + params.AEnd, std::back_inserter(logI), [](double i)
			{ return std::log(i); });

		auto derivate = [&](const std::vector<double>& voltage, const std::vector<double>& current, std::pair<std::vector<double>, std::vector<double>>& result)
			{
				double der{ 0.0 };

				result.first.clear();
				result.second.clear();

				for (size_t i = 0; i < current.size() - 2; i++)
				{
					der = (current[i + 2] - current[i]) / (voltage[i + 2 + params.AStart] - voltage[i + params.AStart]);
					result.first.push_back(voltage[i + 1 + params.AStart]);
					result.second.push_back(der);
				}
			};


		std::pair<std::vector<double>, std::vector<double>> ADerivative;
		derivate(V, logI, ADerivative);


		double maxDer{ 0.0 };
		size_t maxDerIndex{ 0 };
		for (size_t i = 0; i < ADerivative.second.size(); i++)
			if (ADerivative.second[i] > maxDer)
			{
				maxDer = ADerivative.second[i];
				maxDerIndex = i;
			}

		size_t dVB{ maxDerIndex };
		size_t dVE{ maxDerIndex };
		double tol{ 0.95 };
		while (ADerivative.second[dVB] > maxDer * tol || ADerivative.second[dVE] > maxDer * tol)
		{
			if (ADerivative.second[dVB] > maxDer * tol)
				dVB--;
			if (ADerivative.second[dVE] > maxDer * tol)
				dVE++;
			// assert(dVB < ADerivative.second.size());
			if (dVB > ADerivative.second.size() - 1 || dVE > ADerivative.second.size() - 1)
				break;
		}
		const double k = 8.6e-5;
		double dV = V[params.AStart + dVE] - V[params.AStart + dVB];
		params.logI = logI;
		params.maxDerIndex = maxDerIndex;
		params.dV = dV;
		params.k = k;
		return 1 / (k * params.T * maxDer);

	
	}
	FittingService::ParameterMap estimate4PModel(const FittingService::EstimateInput& input,const std::vector<std::pair<double,double>>& AMultipiers)
	{
		std::vector<std::vector<double>> result(2);
		result[0] = std::vector<double>{ input.characteristic.voltageData.begin(), input.characteristic.voltageData.end() };
		result[1].resize(input.characteristic.currentData.size());
		double sum = 0;
		const unsigned int N{ 4 };
		for (size_t i = 0; i < result[1].size(); i++)
		{
			sum += input.characteristic.currentData[i];
			if (i >= N)
			{
				sum -= input.characteristic.currentData[i - N];
			}
			result[1][i] = sum / std::min(i + 1, (size_t)N);
		}

		auto& V = result[0];
		auto& I = result[1];
		Fitters::ParameterMap parameterResult{};
		int RpStart{ 0 }, RsStart{ 0 };
		int AStart{ 0 }, AEnd{ 0 };
		RsStart = I.size() - 1;

		parameterResult[Fitters::ParameterID::Rsh] = estimateRsh(V,I,AStart);
		parameterResult[Fitters::ParameterID::Rs] = estimateRs(V,I,AStart,AEnd);

		// getting A
		double T = input.additionalParameters.at(Fitters::AdditionalParametersID::Temperature);
		IdealityFactorAdditionalParameters idealityFactorParams { AStart, AEnd, 0, T};
		
		double A = estimateIdealityFactor(V, I, idealityFactorParams) * adjustCoefficient(idealityFactorParams.dV, AMultipiers);
		auto logI = idealityFactorParams.logI;
		auto maxDerIndex = idealityFactorParams.maxDerIndex;
		auto k = idealityFactorParams.k;
		double l = logI[maxDerIndex] - V[maxDerIndex] / (A * k * T);
		double l1 = logI[maxDerIndex] - V[maxDerIndex + AStart] / (A * k * T);
		//Info() << "l: " << std::exp(l) << "l1: " << std::exp(l1) << std::endl;
		double I0 = std::exp(l);

		parameterResult[Fitters::ParameterID::A] = A;
		parameterResult[Fitters::ParameterID::I0] = I0;
		return parameterResult;
	}
	// Dark IVs PreFitters
	FourParameterModelPreFit::FourParameterModelPreFit()
		:AbstractPreFit{}{};
	FittingService::ParameterMap FourParameterModelPreFit::Estimate(const FittingService::EstimateInput& input)
	{
		return estimate4PModel(input,m_AMultiplier);
	}

	SixParameterModelPreFit::SixParameterModelPreFit()
		:AbstractPreFit{}{};
	FittingService::ParameterMap SixParameterModelPreFit::Estimate(const FittingService::EstimateInput& input)
	{
		ParameterMap parameterResult = estimate4PModel(input, m_AMultiplier);
		parameterResult[Fitters::ParameterID::alpha] = 2.0;
		parameterResult[Fitters::ParameterID::Rsh2] = parameterResult[Fitters::ParameterID::Rsh];
		return parameterResult;
	}


	//Light IVs IVs PreFitters
	FiveParameterModelPreFit::FiveParameterModelPreFit()
	:AbstractPreFit{}{};

	ParameterMap FiveParameterModelPreFit::Estimate(const FittingService::EstimateInput& input)
	{
		ParameterMap parameterResult = estimate4PModel(input, m_AMultiplier);
		int index = 0;
		if(input.characteristic.voltageData[0]<0)
            int index = std::distance(input.characteristic.voltageData.begin(), std::ranges::find(input.characteristic.voltageData.begin(), input.characteristic.voltageData.end(), 0));
		parameterResult[Fitters::ParameterID::I_sc] = input.characteristic.currentData[index];
		return parameterResult;
	};
	
	
	SevenParameterModelPreFit::SevenParameterModelPreFit()
	:AbstractPreFit{}{};
	ParameterMap SevenParameterModelPreFit::Estimate(const FittingService::EstimateInput& input)
	{
		ParameterMap parameterResult = estimate4PModel(input, m_AMultiplier);
		int index = *std::ranges::find(input.characteristic.voltageData, 0);
		parameterResult[Fitters::ParameterID::I_sc] = input.characteristic.currentData[index];
		return parameterResult;
	};



	PreFitter::PreFitter()
	{
		preFitterMap[Fitters::JFMModelID::Model4P] = std::make_shared<FourParameterModelPreFit>();
		preFitterMap[Fitters::JFMModelID::Model6P] = std::make_shared<SixParameterModelPreFit>();
		preFitterMap[Fitters::JFMModelID::Model4PLight] = std::make_shared<FiveParameterModelPreFit>();
		preFitterMap[Fitters::JFMModelID::Model6PLight] = std::make_shared<SevenParameterModelPreFit>();
	};
	FittingService::ParameterMap PreFitter::Estimate(const FittingService::EstimateInput& input)
	{
		return preFitterMap[input.modelID]->Estimate(input);
	}
	std::shared_ptr<AbstractPreFit> PreFitter::operator[](const FittingService::ModelID id)
	{
		return preFitterMap[id];
	}
	;
	std::pair<size_t, size_t> PreFitter::RangeData(const FittingService::PlotData& characteristic)
	{
		return preFitterMap[Fitters::JFMModelID::Model4P]->RangeData(characteristic); //FIX it to be more general
	};
}
