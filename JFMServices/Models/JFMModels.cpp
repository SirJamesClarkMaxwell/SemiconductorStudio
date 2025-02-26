#include "JFMModels.hpp"
namespace JFMService
{
    template <size_t parameter_size>
    std::array<double, parameter_size> adjustFixingConfiguration(NumericStorm::Fitting::Parameters<parameter_size> &parameters, JFMAdditionalParameters &additionalParameters)
    {
        std::array<double, parameter_size> destination(parameters.getParameters());
        std::valarray<double> fixedValues(additionalParameters.fixingValues.getParameters());
        FixingConfiguration config = additionalParameters.fixingConfiguration;
        for (const auto &[dst, src] : std::views::zip(destination, fixedValues))
        {
            if (config & 1)
                dst = src;
            config >>= (uint32_t)1;
        }
        return destination;
    };

    FourParameterModel::FourParameterModel()
    {
        m_model = std::bind(&FourParameterModel::current, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    }

    void FourParameterModel::current(Data &data, const NumericStorm::Fitting::Parameters<4> &parameters, const JFMAdditionalParameters &additionalParameters)
    {
        // I(V) = I0(e^(q(V-IRs)/AKT)-1) - (V - IRs)/Rsh
        NumericStorm::Fitting::Parameters<4> params(parameters);
        JFMAdditionalParameters additionalParams(additionalParameters);
        auto adjusted = adjustFixingConfiguration<4>(params, additionalParams);
        auto [I0,A,  Rs, Rsh] = adjusted;
        const double k = 8.6e-5;

        auto func = [&](double &V, double &I, double &I0, double &A, double &Rsh, double &Rs, double T)
        {

            double x = ((I0 * Rs) / (A * k * T)) * std::exp(V / (A * k * T));
            double I_lw = utl::LambertW<0>(x);
            I_lw *= (A * k * T) / Rs;
            I = I_lw + (V - I_lw * Rs) / Rsh;
            
        };

        for (const auto &[V, I] : std::views::zip(data[0], data[1]))
        {
       /*     if (std::any_of(parameters.begin(), parameters.end(), [&](double item) {return item < 0.0;}))
                I = 1 / I;
            else*/
                func(V, I, I0, A, Rsh, Rs, additionalParameters.Temperature);
        }
    }
    void FourParameterModel::call( CalculatingData& data)
    {
        NumericStorm::Fitting::Data NSData(2);
        NSData[0] = std::vector<double>{data.characteristic.voltageData.begin(),data.characteristic.voltageData.end()};
        NSData[1] = std::vector<double>{ data.characteristic.currentData.begin(),data.characteristic.currentData.end() };
        NumericStorm::Fitting::Parameters<4> params;
        for (const auto& [id, val] : data.parameters)
            params[id] = val;
        JFMAdditionalParameters additional;
        additional.Temperature = (*data.additionalParameters.begin()).second;
        current(NSData, params, additional);
        for (const auto& [dst, src] : std::views::zip(data.characteristic.currentData, NSData[1]))
            dst = src;
        for (const auto& [dst, src] : std::views::zip(data.characteristic.voltageData, NSData[0]))
            dst = src;
    };

    SixParameterModel::SixParameterModel()
    {
        m_model = std::bind(&SixParameterModel::current, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    }
    void SixParameterModel::current(Data &data, const NumericStorm::Fitting::Parameters<6> &parameters, const JFMAdditionalParameters &additionalParameters)
    {
        // I(V) = I0(e^(q(V-IRs)/AKT)-1) - (V - IRs)/Rsh - (V - IRs)^alpha/Rsh2
        NumericStorm::Fitting::Parameters<6> params(parameters);
        JFMAdditionalParameters additionalParams(additionalParameters);
        auto adjusted = adjustFixingConfiguration<6>(params, additionalParams);
        auto [ I0,A, Rs, Rsh, alpha, Rsh2] = adjusted; // utils::cast<6>(adjusted);
        const double k = 8.6e-5;

        auto func = [&](double &V, double &I, double &I0, double &A, double &Rsh, double &Rs, double &alpha, double &Rsh2, double T)
        {
            double x = ((I0 * Rs) / (A * k * T)) * std::exp(V / (A * k * T));
            double I_lw = utl::LambertW<0>(x);
            I_lw *= (A * k * T) / Rs;
            double additionalFactor = std::pow((V - I_lw * Rs), alpha) / Rsh2;
            I = I_lw + (V - I_lw * Rs) / Rsh + additionalFactor;
        };

        for (const auto &[V, I] : std::views::zip(data[0], data[1]))
            func(V, I, I0, A, Rsh, Rs, alpha, Rsh2, additionalParameters.Temperature);
    }
    void SixParameterModel::call( CalculatingData& data)
    {
        NumericStorm::Fitting::Data NSData(2);
        NSData[0] = std::vector<double>{ data.characteristic.voltageData.begin(),data.characteristic.voltageData.end() };
        NSData[1] = std::vector<double>{ data.characteristic.currentData.begin(),data.characteristic.currentData.end() };
        NumericStorm::Fitting::Parameters<6> params;
        for (const auto& [id, val] : data.parameters)
            params[id] = val;
        JFMAdditionalParameters additional;
        additional.Temperature = (*data.additionalParameters.begin()).second;
        current(NSData, params, additional);
        for (const auto& [dst, src] : std::views::zip(data.characteristic.currentData, NSData[1]))
            dst = src;
        for (const auto& [dst, src] : std::views::zip(data.characteristic.voltageData, NSData[0]))
            dst = src;
    };

}