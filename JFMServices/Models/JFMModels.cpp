#include "JFMModels.hpp"
namespace JFMService
{
    template <size_t parameter_size>
    std::array<double, parameter_size> adjustFixingConfiguration(NumericStorm::Fitting::Parameters<parameter_size> &parameters, JFMAdditionalParameters &additionalParameters)
    {
        std::array<double, parameter_size> destination(parameters.getParameters());
        std::valarray<double> fixedValues(additionalParameters.fixingValues.getParameters());
        FixingConfiguration config = additionalParameters.fixingConfiguration;
        int i = 0;
        for (const auto &[dst, src] : std::views::zip(destination, fixedValues))
        {
            if (config & 1)
                destination[i] = src;
            i++;
            config >>= (uint32_t)1;
        }
        return destination;
    };

    template <size_t parameter_size, class CurrentModel>
    void call_model(CalculatingData &data, CurrentModel model)
    {
        NumericStorm::Fitting::Data NSData(2);
        NSData[0] = std::vector<double>{data.characteristic.voltageData.begin(), data.characteristic.voltageData.end()};
        NSData[1] = std::vector<double>{data.characteristic.currentData.begin(), data.characteristic.currentData.end()};
        NumericStorm::Fitting::Parameters<parameter_size> params;
        for (const auto &[id, val] : data.parameters)
            params[id] = val;
        JFMAdditionalParameters additional;
        additional.Temperature = (*data.additionalParameters.begin()).second;
        model(NSData, params, additional);
        for (const auto &[dst, src] : std::views::zip(data.characteristic.currentData, NSData[1]))
            dst = src;
        for (const auto &[dst, src] : std::views::zip(data.characteristic.voltageData, NSData[0]))
            dst = src;
    }

    //! Dark IVs Model
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
        auto [I0, A, Rs, Rsh] = adjusted;
        const double k = 8.6e-5;

        auto func = [&](double &V, double &I, double &I0, double &A, double &Rsh, double &Rs, double T)
        {
            double x = ((I0 * Rs) / (A * k * T)) * std::exp(V / (A * k * T));
            double I_lw = utl::LambertW<0>(x);
            I_lw *= (A * k * T) / Rs;
            I = I_lw + (V - I_lw * Rs) / Rsh;
        };

        for (const auto &[V, I] : std::views::zip(data[0], data[1]))
            func(V, I, I0, A, Rsh, Rs, additionalParameters.Temperature);
    }
    void FourParameterModel::call(CalculatingData &data)
    {
        call_model<4>(data, [this](Data &d, const NumericStorm::Fitting::Parameters<4> &p, const JFMAdditionalParameters &a)
                      { this->current(d, p, a); });
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
        auto [I0, A, Rs, Rsh, alpha, Rsh2] = adjusted; // utils::cast<6>(adjusted);
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
    void SixParameterModel::call(CalculatingData &data)
    {
        call_model<6>(data, [this](Data &d, const NumericStorm::Fitting::Parameters<6> &p, const JFMAdditionalParameters &a)
                      { this->current(d, p, a); });
    };

    //! light IVs models

    FiveParameterModel::FiveParameterModel()
    {
        m_model = std::bind(&FiveParameterModel::current, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    }

    void FiveParameterModel::current(Data &data, const NumericStorm::Fitting::Parameters<5> &parameters, const JFMAdditionalParameters &additionalParameters)
    {
        // I(V) = I0(e^(q(V-IRs)/AKT)-1) - (V - IRs)/Rsh
        NumericStorm::Fitting::Parameters<5> params(parameters);
        JFMAdditionalParameters additionalParams(additionalParameters);
        auto adjusted = adjustFixingConfiguration<5>(params, additionalParams);
        auto [I0, A, Rs, Rsh, Isc] = adjusted;
        const double k = 8.6e-5;

        auto func = [&](double &V, double &I, double &I0, double &A, double &Rsh, double &Rs, double &Isc, double T)
        {
            double x = ((I0 * Rs) / (A * k * T)) * std::exp(V / (A * k * T));
            double I_lw = utl::LambertW<0>(x);
            I_lw *= (A * k * T) / Rs;
            I = I_lw + (V - I_lw * Rs) / Rsh; //+ Isc;
        };

        for (const auto &[V, I] : std::views::zip(data[0], data[1]))
            func(V, I, I0, A, Rsh, Rs, Isc, additionalParameters.Temperature);
    }

    void FiveParameterModel::call(CalculatingData &data)
    {
        call_model<5>(data, [this](Data &d, const NumericStorm::Fitting::Parameters<5> &p, const JFMAdditionalParameters &a)
                      { this->current(d, p, a); });
    }
    SevenParameterModel::SevenParameterModel()
    {
        m_model = std::bind(&SevenParameterModel::current, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    }
    void SevenParameterModel::current(Data &data, const NumericStorm::Fitting::Parameters<7> &parameters, const JFMAdditionalParameters &additionalParameters)
    {
        // I(V) = I0(e^(q(V-IRs)/AKT)-1) - (V - IRs)/Rsh - (V - IRs)^alpha/Rsh2 - Isc
        NumericStorm::Fitting::Parameters<7> params(parameters);
        JFMAdditionalParameters additionalParams(additionalParameters);
        auto adjusted = adjustFixingConfiguration<7>(params, additionalParams);
        auto [I0, A, Rs, Rsh, alpha, Rsh2, Isc] = adjusted; // utils::cast<6>(adjusted);
        const double k = 8.6e-5;

        auto func = [&](double &V, double &I, double &I0, double &A, double &Rsh, double &Rs, double &alpha, double &Rsh2, double &Isc, double T)
        {
            double x = ((I0 * Rs) / (A * k * T)) * std::exp(V / (A * k * T));
            double I_lw = utl::LambertW<0>(x);
            I_lw *= (A * k * T) / Rs;
            double additionalFactor = std::pow((V - I_lw * Rs), alpha) / Rsh2;
            I = I_lw + (V - I_lw * Rs) / Rsh + additionalFactor; // +Isc;
        };

        for (const auto &[V, I] : std::views::zip(data[0], data[1]))
            func(V, I, I0, A, Rsh, Rs, alpha, Rsh2, Isc, additionalParameters.Temperature);
    }
    void SevenParameterModel::call(CalculatingData &data)
    {
        call_model<7>(data, [this](Data &d, const NumericStorm::Fitting::Parameters<7> &p, const JFMAdditionalParameters &a)
                      { this->current(d, p, a); });
    }
}