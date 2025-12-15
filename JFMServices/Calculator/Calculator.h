#pragma once

#include <ranges>
#include <vector>

#include "Fitting/JFMIFitting.hpp"
#include "Fitting/JFMFitter.hpp"
#include "Fitting/PreFitter.hpp"
#include "utils.hpp"
#include "SymbolLoader/SymbolLoader.h"
#include "SimulationEngine.hpp"

using namespace JFMService::Fitters;
using JFMService::SimulationEngine;

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
    const MCInput &input;
    mutable std::vector<MCResult> *output;
    SimulationEngine *engine;

    CalculationParams(
            const MCInput &input,
            std::vector<MCResult> *output,
            SimulationEngine *engine)
        : input(input)
        , output(output)
        , engine(engine)
    {
    }
};

struct CalculationParamsNonSimulate : public CalculationParams
{
    // For simplicity we keep cartesian product object a member.
    // Due to it being a reference to pSets objects we need to keep
    // pSets within _this_ object so as to provide sufficient lifetime
    // of cartesian object
    mutable std::vector<std::vector<double>> pSets;
    ProductT cartesian;
    size_t totalLength;

    CalculationParamsNonSimulate(
            const MCInput &input,
            std::vector<MCResult> *output,
            SimulationEngine *engine,
            std::vector<std::vector<double>> pSets)
        : CalculationParams(input, output, engine)
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
            SimulationEngine *engine,
            std::vector<std::vector<double>> pSets)
        : CalculationParamsNonSimulate(input, output, engine, pSets)
    {
    }
};

struct CalculationParamsCpuMulti : public CalculationParamsNonSimulate
{
    CalculationParamsCpuMulti(
            const MCInput &input,
            std::vector<MCResult> *output,
            SimulationEngine *engine,
            std::vector<std::vector<double>> pSets)
        : CalculationParamsNonSimulate(input, output, engine, pSets)
    {
    }
};

struct CalculationParamsGpu : public CalculationParamsNonSimulate
{
    CalculationParamsGpu(
            const MCInput &input,
            std::vector<MCResult> *output,
            SimulationEngine *engine,
            std::vector<std::vector<double>> pSets)
        : CalculationParamsNonSimulate(input, output, engine, pSets)
    {
    }
};

struct CalculationParamsSimulate : public CalculationParams
{
    std::shared_ptr<Fitters::AbstractFitter> fitter;
    std::shared_ptr<AbstractPreFit> preFitter;

    CalculationParamsSimulate(
            const MCInput &input,
            std::vector<MCResult> *output,
            SimulationEngine *engine,
            std::shared_ptr<Fitters::AbstractFitter> fitter,
            std::shared_ptr<AbstractPreFit> preFitter)
        : CalculationParams(input, output, engine)
        , fitter(fitter)
        , preFitter(preFitter)
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
private:
    constexpr static const char *library_path = "libcalculator_gpu.so";
    static utils::SymbolLoader *loader;

public:
    static void call(const CalculationParamsGpu &params);
};

void fillUpResultOutputs(
    const MCInput &input,
    std::vector<MCResult> *output,
    const ProductT &cartesian);
} // namespace Calculator
