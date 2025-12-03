#pragma once
#include "pch.hpp"
#include "../Fitting/JFMIFitting.hpp"
#include "../Models/JFMAdditionalParameters.hpp"

namespace JFMService::Fitters
{

    //enum ParameterID
    //{
    //    I0=0,
    //    A=1,
    //    Rs,
    //    Rsh,
    //    alpha,
    //    Rsh2,
    //    I_sc,
    //    p_size
    //};
    //enum AdditionalParametersID
    //{
    //    Temperature = p_size,
    //};

    using namespace FittingService;
    using Callback = std::function<void(ParameterMap &&)>;

    class AbstractFitter
    {
    public:
        virtual ~AbstractFitter() = default;
        virtual void Fit(const FittingInput &input, Callback callback) = 0;
    };

    class FourParameterFitter : public AbstractFitter
    {
    public:
        FourParameterFitter() = default;
        virtual void Fit(const FittingInput &input, Callback callback) override;
    };
    class FiveParameterFitter : public AbstractFitter
    {
    public:
        FiveParameterFitter() = default;
        virtual void Fit(const FittingInput &input, Callback callback) override;
    };
    class SevenParameterFitter : public AbstractFitter
    {
    public:
        SevenParameterFitter() = default;
        virtual void Fit(const FittingInput &input, Callback callback) override;
    };
    class SixParameterFitter : public AbstractFitter
    {
    public:
        SixParameterFitter() = default;
        virtual void Fit(const FittingInput &input, Callback callback) override;
    };

    class Fitter
    {
        using FitterMap = std::unordered_map<ModelID, std::shared_ptr<AbstractFitter>>;

    public:
        Fitter();
        void Fit(const FittingInput &input, Callback callback);
        std::shared_ptr<AbstractFitter> operator[](ModelID id);

    private:
        FitterMap fitterMap{};
    };

}
