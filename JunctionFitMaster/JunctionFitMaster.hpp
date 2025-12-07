#pragma once

#include "utils.hpp"
#include "SymbolLoader/SymbolLoader.h"

namespace JunctionFitMaster
{
constexpr const utils::CalculationModeId modeId =
#if defined(JFM_ITER_SIMULATE)
    utils::CalculationModeId::CalculateSimulate;
#elif defined(JFM_MODE_MULTI_CORE)
    utils::CalculationModeId::CalculateMultiCore;
#elif defined(JFM_MODE_SINGLE_CORE)
    utils::CalculationModeId::CalculateSingleCore;
#elif defined(JFM_MODE_GPU)
    utils::CalculationModeId::CalculateGpu;
#else
#error "Invalid option. Use one of modes : { 'single-cpu' , 'multi-cpu' ,'gpu', 'simulate' }"
#endif

constexpr const utils::PlatformType platformType =
#if defined(JFM_PLATFORM_NIX)
    utils::PlatformType::Nix;
#else
    utils::Platform::Windows;
#endif
}
