#include "PreFitter.hpp"
#include "JFMFitter.hpp"
#include "CalculateData.hpp"




namespace JFMService
{

	std::pair<size_t, size_t> AbstractPreFit::RangeData(const FittingService::PlotData& characteristic)
	{
		return std::make_pair<size_t, size_t>(getLowerRange(characteristic), getUpperRange(characteristic));
	};
	size_t AbstractPreFit::getLowerRange(const FittingService::PlotData& characteristic)
	{
		size_t start{ 0 };
		for (const auto& [V, I] : std::views::zip(characteristic.voltageData, characteristic.voltageData))
			if (I < 0.0 or V < 0.0)
				start++;
		start += 2;
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

	FourParameterModelPreFit::FourParameterModelPreFit()
		:AbstractPreFit{}
	{
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
		m_AMultiplier.resize(dV.size());
		for (const auto& [dest, dv, coeff] : std::views::zip(m_AMultiplier, dV, alpha))
			dest = { dv, coeff };
		std::sort(m_AMultiplier.begin(), m_AMultiplier.end(), [&](std::pair<double, double> lhs, std::pair<double, double> rhs)
			{ return lhs.first < rhs.first; });
	};
	double FourParameterModelPreFit::adjustCoefficient(double dV)
	{
		int index = std::distance(m_AMultiplier.begin(), std::lower_bound(m_AMultiplier.begin(), m_AMultiplier.end(), dV, [&](std::pair<double, double> lhs, double rhs)
			{ return lhs.first < rhs; }));
		auto interpolate = [&](int index)
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
	};
	FittingService::ParameterMap FourParameterModelPreFit::Estimate(const FittingService::EstimateInput& input)
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

		size_t RpStart{ 0 }, RsStart{ 0 };
		size_t AStart{ 0 }, AEnd{ 0 };



		RsStart = I.size() - 1;

		double S{ 0.0 }, S0{ 0.0 };

		// getting Rp
		size_t n{ RpStart + 2 };
		do
		{
			if (n >= V.size() || n >= I.size() || I.size() == 0 || V.size() == 0)
				break;

			S = (V[n - 1] - V[n - 2]) / (I[n - 1] - I[n - 2]);
			S0 = (V[n - 1] - V[RpStart]) / (I[n - 1] - I[RpStart]);

			n++;
		} while (S / S0 >= 0.8);

		AStart = n;

		parameterResult[Fitters::ParameterID::Rsh] = S0;

		// getting Rs
		n = RsStart - 2;
		do
		{
			if (n == 0 || n == AStart)
				break;

			S = (V[n + 2] - V[n + 1]) / (I[n + 2] - I[n + 1]);
			S0 = (V[RsStart] - V[n + 1]) / (I[RsStart] - I[n + 1]);

			n--;
		} while (S / S0 <= 0.8);

		AEnd = n;

		parameterResult[Fitters::ParameterID::Rs] = S0;

		// getting A

		std::vector<double> logI{};

		std::transform(I.begin() + AStart, I.begin() + AEnd, std::back_inserter(logI), [](double i)
			{ return std::log(i); });

		auto derivate = [&](const std::span<double>& voltage, const std::span<double>& current, std::pair<std::vector<double>, std::vector<double>>& result)
			{
				double der{ 0.0 };

				result.first.clear();
				result.second.clear();

				for (size_t i = 0; i < current.size() - 2; i++)
				{
					der = (current[i + 2] - current[i]) / (voltage[i + 2 + AStart] - voltage[i + AStart]);
					result.first.push_back(voltage[i + 1 + AStart]);
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

		double dV = V[AStart + dVE] - V[AStart + dVB];

		const double k = 8.6e-5;
		double T = input.additionalParameters.at(Fitters::AdditionalParametersID::Temperature);
		double A = 1 / (k * T * maxDer);

		A *= adjustCoefficient(dV);
		double l = logI[maxDerIndex] - V[maxDerIndex] / (A * k * T);
		double l1 = logI[maxDerIndex] - V[maxDerIndex + AStart] / (A * k * T);
		std::cout << "l: " << std::exp(l) << "l1: " << std::exp(l1) << std::endl;
		double I0 = std::exp(l);

		parameterResult[Fitters::ParameterID::A] = A;
		parameterResult[Fitters::ParameterID::I0] = I0;

		return parameterResult;
	}



	SixParameterModelPreFit::SixParameterModelPreFit()
		:AbstractPreFit{}
	{
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
		m_AMultiplier.resize(dV.size());
		for (const auto& [dest, dv, coeff] : std::views::zip(m_AMultiplier, dV, alpha))
			dest = { dv, coeff };
		std::sort(m_AMultiplier.begin(), m_AMultiplier.end(), [&](std::pair<double, double> lhs, std::pair<double, double> rhs)
			{ return lhs.first < rhs.first; });
	};

	double SixParameterModelPreFit::adjustCoefficient(double dV)
	{
		int index = std::distance(m_AMultiplier.begin(), std::lower_bound(m_AMultiplier.begin(), m_AMultiplier.end(), dV, [&](std::pair<double, double> lhs, double rhs)
			{ return lhs.first < rhs; }));
		auto interpolate = [&](int index)
			{
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
	};
	FittingService::ParameterMap SixParameterModelPreFit::Estimate(const FittingService::EstimateInput& input)
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

		size_t RpStart{ 0 }, RsStart{ 0 };
		size_t AStart{ 0 }, AEnd{ 0 };



		RsStart = I.size() - 1;

		double S{ 0.0 }, S0{ 0.0 };

		// getting Rp
		size_t n{ RpStart + 2 };
		do
		{
			if (n >= V.size() || n >= I.size() || I.size() == 0 || V.size() == 0)
				break;

			S = (V[n - 1] - V[n - 2]) / (I[n - 1] - I[n - 2]);
			S0 = (V[n - 1] - V[RpStart]) / (I[n - 1] - I[RpStart]);

			n++;
		} while (S / S0 >= 0.8);

		AStart = n;

		parameterResult[Fitters::ParameterID::Rsh] = S0;

		// getting Rs
		n = RsStart - 2;
		do
		{
			if (n == 0 || n == AStart)
				break;

			S = (V[n + 2] - V[n + 1]) / (I[n + 2] - I[n + 1]);
			S0 = (V[RsStart] - V[n + 1]) / (I[RsStart] - I[n + 1]);

			n--;
		} while (S / S0 <= 0.8);

		AEnd = n;

		parameterResult[Fitters::ParameterID::Rs] = S0;

		// getting A

		std::vector<double> logI{};

		std::transform(I.begin() + AStart, I.begin() + AEnd, std::back_inserter(logI), [](double i)
			{ return std::log(i); });

		auto derivate = [&](const std::span<double>& v, const std::span<double>& c, std::pair<std::vector<double>, std::vector<double>>& result)
			{
				double der{ 0.0 };

				result.first.clear();
				result.second.clear();

				for (size_t i = 0; i < c.size() - 2; i++)
				{
					der = (c[i + 2] - c[i]) / (v[i + 2 + AStart] - v[i + AStart]);
					result.first.push_back(v[i + 1 + AStart]);
					result.second.push_back(der);
				}
			};

		auto derivateFull = [&](const std::span<double>& v, const std::span<double>& c, std::pair<std::vector<double>, std::vector<double>>& result)
			{
				double der{ 0.0 };

				result.first.clear();
				result.second.clear();

				for (size_t i = 0; i < c.size() - 2; i++)
				{
					der = (c[i + 2] - c[i]) / (v[i + 2] - v[i]);
					result.first.push_back(v[i + 1]);
					result.second.push_back(der);
				}
			};

		std::pair<std::vector<double>, std::vector<double>> ADerivative;
		derivate(V, logI, ADerivative);

		std::vector<double> filteredI = { I.begin(), I.end() };

		std::vector<double> logV{}, loglogI{};
		std::transform(V.begin(), V.end(), std::back_inserter(logV), [](double v)
			{ return std::log(v); });

		std::transform(filteredI.begin(), filteredI.end(), std::back_inserter(loglogI), [](double i)
			{ return std::log(i); });

		/*filteredI.clear();
		for (const auto& [i, v] : std::views::zip(I, V)) {
			filteredI.push_back(std::log(i - v / parameterResult[Fitters::ParameterID::Rsh]));
		}*/

		//globalErrors.resize(2);
		//derivateFull(logV, loglogI, globalErrors[0]);
		//globalErrors[1].first = logV;
		//globalErrors[1].second = loglogI;
		/*for (const auto& [i, v] : std::views::zip(loglogI, logV))
			globalErrors[1].second.push_back((i - std::log(parameterResult[Fitters::ParameterID::Rsh])) / v);*/






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

		double dV = V[AStart + dVE] - V[AStart + dVB];

		const double k = 8.6e-5;
		double T = input.additionalParameters.at(Fitters::AdditionalParametersID::Temperature);
		double A = 1 / (k * T * maxDer);

		A *= adjustCoefficient(dV);
		// log(I) = V / (A  * k * T) + log(I0)
		// I0 = I / exp(V / (A * k * T))
		// double I0 = I[maxDerIndex + AStart] / std::exp(V[maxDerIndex + AStart] / (A * k * T));
		double l = logI[maxDerIndex] - V[maxDerIndex + AStart] / (A * k * T);

		double I0 = std::exp(l);

		parameterResult[Fitters::ParameterID::A] = A;
		parameterResult[Fitters::ParameterID::I0] = I0;

		parameterResult[Fitters::ParameterID::alpha] = 2.0;
		parameterResult[Fitters::ParameterID::Rsh2] = parameterResult[Fitters::ParameterID::Rsh];

		return parameterResult;
	}




	PreFitter::PreFitter()
	{

		preFitterMap[Model4P] = std::make_shared<FourParameterModelPreFit>();
		preFitterMap[Model6P] = std::make_shared<SixParameterModelPreFit>();
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
		return preFitterMap[Model4P]->RangeData(characteristic);
	};
}