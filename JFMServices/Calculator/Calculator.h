#pragma once

#include <ranges>
#include <vector>

#include "Fitting/JFMIFitting.hpp"
#include "Fitting/JFMFitter.hpp"
#include "Fitting/PreFitter.hpp"
#include "utils.hpp"

using namespace JFMService::Fitters;

using ProductT = std::ranges::cartesian_product_view<
    std::views::all_t<std::vector<double> &>,
    std::views::all_t<std::vector<double> &>,
    std::views::all_t<std::vector<double> &>,
    std::views::all_t<std::vector<double> &> >;


namespace Calculator
{
using namespace JFMService;

struct CalculationParams
{
    using CalculationCb = void (*)(const MCInput& input, MCResult& result);

    const MCInput &input;
    mutable std::vector<MCResult> *output;
    CalculationCb cb;

    CalculationParams(
            const MCInput &input,
            std::vector<MCResult> *output,
            CalculationCb cb)
        : input(input)
        , output(output)
        , cb(cb)
    {
    }
};

struct CalculationParamsNonSimulate : public CalculationParams
{
    // For simplicity we keep cartesian product object a member.
    // Due to it being a reference to pSets objects we need to keep
    // pSets within _this_ object so as to provide sufficient lifetime
    // of cartesian object
    std::vector<std::vector<double>> pSets;
    ProductT cartesian;
    size_t totalLength;

    CalculationParamsNonSimulate(
            const MCInput &input,
            std::vector<MCResult> *output,
            CalculationCb cb,
            std::vector<std::vector<double>> pSets)
        : CalculationParams(input, output, cb)
        , pSets(pSets)
        , cartesian(std::views::cartesian_product(this->pSets[0], this->pSets[1], this->pSets[2], this->pSets[3]))
        , totalLength(cartesian.size())
    {
        this->output->resize(this->totalLength);
    }
};

struct CalculationParamsCpuSingle : public CalculationParamsNonSimulate
{
    CalculationParamsCpuSingle(
            const MCInput &input,
            std::vector<MCResult> *output,
            CalculationCb cb,
            std::vector<std::vector<double>> pSets)
        : CalculationParamsNonSimulate(input, output, cb, pSets)
    {
    }
};

struct CalculationParamsCpuMulti : public CalculationParamsNonSimulate
{
    CalculationParamsCpuMulti(
            const MCInput &input,
            std::vector<MCResult> *output,
            CalculationCb cb,
            std::vector<std::vector<double>> pSets)
        : CalculationParamsNonSimulate(input, output, cb, pSets)
    {
    }
};

struct CalculationParamsGpu : public CalculationParamsNonSimulate
{
    CalculationParamsGpu(
            const MCInput &input,
            std::vector<MCResult> *output,
            CalculationCb cb,
            std::vector<std::vector<double>> pSets)
        : CalculationParamsNonSimulate(input, output, cb, pSets)
    {
    }
};

struct CalculationParamsSimulate : public CalculationParams
{
    using GenerateNoiseCb = void (*)(double &value, double factor);

    std::shared_ptr<Fitters::AbstractFitter> fitter;
    std::shared_ptr<AbstractPreFit> preFitter;
    GenerateNoiseCb generateNoiseCb;

    CalculationParamsSimulate(
            const MCInput &input,
            std::vector<MCResult> *output,
            CalculationCb cb,
            std::shared_ptr<Fitters::AbstractFitter> fitter,
            std::shared_ptr<AbstractPreFit> preFitter,
            GenerateNoiseCb generateNoiseCb)
        : CalculationParams(input, output, cb)
        , fitter(fitter)
        , preFitter(preFitter)
        , generateNoiseCb(generateNoiseCb)
    {
    }
};

template <typename CalculationParamsAll>
struct CalculatorAll;

template <>
struct CalculatorAll<CalculationParamsSimulate>
{
    static void call(const CalculationParamsSimulate &params);
};

template <>
struct CalculatorAll<CalculationParamsCpuSingle>
{
    static void call(const CalculationParamsCpuSingle &params);
};

template <>
struct CalculatorAll<CalculationParamsCpuMulti>
{
    static void call(const CalculationParamsCpuMulti &params);
};

template <>
struct CalculatorAll<CalculationParamsGpu>
{
    static void call(const CalculationParamsGpu &params);
};
} // namespace Calculator
