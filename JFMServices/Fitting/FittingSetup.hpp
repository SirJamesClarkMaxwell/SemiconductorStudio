#pragma once
#include "NumericStorm.hpp"
#include "SimplexCreatorSettings.hpp"
#include "VisitorOperationBase.hpp"
#include "BasicSimplexOptimizerSettings.hpp"
#include "BasicSimplexOptimizer.hpp"
#include "AdditionalParameters.hpp"
#include "Random.hpp"
#include "Model.hpp"
#include "Optimizer.hpp"
#include "ModelBase.hpp"
#include "Data.hpp"
#include "Fitter.hpp"
#include "Models/JFMModels.hpp"
#include "Models/JFMErrorModel.hpp"

#include "../Fitting/LogDist.hpp"
namespace JFMService
{

	using namespace NumericStorm::Utils;
	using namespace NumericStorm::Fitting;
	using namespace NumericStorm::Concepts;

	template <size_t parameter_size>
	class LogCreatorSettings : public SimplexCreatorSettings<parameter_size>
	{
	public:
		LogCreatorSettings() = default;

		LogCreatorSettings(const NumericStorm::Fitting::Parameters<parameter_size>& min, const NumericStorm::Fitting::Parameters<parameter_size>& max)
			: SimplexCreatorSettings<parameter_size>(min, max) {};

		auto& getP()
		{
			return m_p;
		}

		const auto& getP() const
		{
			return m_p;
		}

		void setP(double p)
		{
			m_p = p;
		}

	private:
		double m_p{ 0.5 };
	};

	template <size_t parameter_size>
	class LogSimplexCreator : public VisitorOperationBase<LogCreatorSettings<parameter_size>>
	{
	public:
		using SettingsT = typename VisitorOperationBase<LogCreatorSettings<parameter_size>>::SettingsT;

		LogSimplexCreator() = default;

		LogSimplexCreator(const SettingsT& settings)
			: VisitorOperationBase<SettingsT>{ settings } {}

		virtual ~LogSimplexCreator() = default;

		typename SettingsT::Out operator()(const typename SettingsT::In& input)
		{
			std::array<typename SettingsT::In, parameter_size + 1> points{};
			points.fill(input);
			points[0].evaluatePoint();

			std::for_each(points.begin() + 1, points.end(), [&](auto& point)
				{
					size_t index{ 0 };
					std::for_each(point.begin(), point.end(), [&](auto& value)
						{
							if (index == 1)
								value = LogDist::value(this->m_settings.getMinBounds()[index], this->m_settings.getMaxBounds()[index]);
							else
								value = Random::Float(this->m_settings.getMinBounds()[index], this->m_settings.getMaxBounds()[index]);
							index++; });
					point.evaluatePoint();
				});
			typename SettingsT::Out figure{ points };
			return figure;
		};
	};

	template <Model M>
	class IVSimplexOptimizerSettings : public BasicSimplexOptimizerSettings<M>
	{
	public:
		IVSimplexOptimizerSettings() = default;

		auto& getLogCreatorSettings()
		{
			return m_logCreatorSettings;
		}

	protected:
		LogCreatorSettings<M::parameter_size> m_logCreatorSettings{};

	protected:
		template <class BuildingType, OptimizerSettings Settings>
#ifdef __clang__
		class IVSimplexOptimizerSettingsBuilderBase : public BasicSimplexOptimizerSettings<M>::template BasicSimplexOptimizerSettingsBuilderBase<BuildingType, Settings>
#else
		class IVSimplexOptimizerSettingsBuilderBase : public BasicSimplexOptimizerSettings<M>::BasicSimplexOptimizerSettingsBuilderBase<BuildingType, Settings>
#endif
		{
		public:
			BuildingType& addLogCreatorSettings(const LogCreatorSettings<M::parameter_size>& settings)
			{
				this->m_settingsObject.m_logCreatorSettings = settings;
				return this->returnSelf();
			}
		};

		friend class IVSimplexOptimizerSettingsBuilder;

	public:
		class IVSimplexOptimizerSettingsBuilder : public IVSimplexOptimizerSettingsBuilderBase<IVSimplexOptimizerSettingsBuilder, IVSimplexOptimizerSettings<M>>
		{
		};
	};

    //  NOTE: stepper can be set by inherited class:
    //      -> BasicSimplexOptimizer
    //      -> SimulatedAnnealzing
	template <Model M>
	class IVSimplexOptimizer : public BasicSimplexOptimizer<IVSimplexOptimizerSettings<M>, void>
	{
	public:
		using SettingsT = IVSimplexOptimizerSettings<M>;
		using AdapterT = void;

		IVSimplexOptimizer() : BasicSimplexOptimizer<IVSimplexOptimizerSettings<M>, void>{ IVSimplexOptimizerSettings<M>{} } {}

		IVSimplexOptimizer(const SettingsT& settings)
			: BasicSimplexOptimizer<IVSimplexOptimizerSettings<M>, void>{ settings } {}

		virtual ~IVSimplexOptimizer() = default;

		void setUp()
		{
			BasicSimplexOptimizer<IVSimplexOptimizerSettings<M>, void>::setUp();
			m_logSimplexCreator.updateSettings(this->m_settings.getLogCreatorSettings());
		};

		typename SettingsT::OptimizerStateT setUpOptimization(const typename SettingsT::OptimizerInputT& input, const Data& data, const typename SettingsT::AuxilaryParametersT& additionalParameters)
		{
			SimplexPoint<M::parameter_size> inputPoint{ input };
			inputPoint.getData() = data;
			inputPoint.onEvaluate([&](SimplexPoint<M::parameter_size>& point)
				{
					this->m_settings.getModel()(point.getData(), point.getParameters(), additionalParameters);
					point.setError(this->m_settings.getErrorModel()(point.getData(), data)); });

			auto pointCount = SimplexStrategySettings<M::parameter_size>::indecies::Count;
			typename SettingsT::OptimizerStateT state{ m_logSimplexCreator(inputPoint), pointCount };

			return state;
		};

	protected:
		LogSimplexCreator<M::parameter_size> m_logSimplexCreator{};
	};

	template <size_t size>
	struct IVFittingSetup
	{
		IVFittingSetup() = default;
		NumericStorm::Fitting::Parameters<size> simplexMin{};
		NumericStorm::Fitting::Parameters<size> simplexMax{};

		// the parameter of the logarithmic distribution for the simplex creator
		double logP{ 0.5 };

		// simplex operations coeficients

		double reflec_coeff{ 1.0 };
		double expand_coeff{ 2.0 };
		double contract_coeff{ 0.5 };
		double shrink_coeff{ 0.5 };
		int numberOfFits{ 1 };
		double minError{ 1e-4 };
		long int maxIteration{ 1000 };
	};

	template <Model M, size_t size>
	IVSimplexOptimizer<M> getOptimizer(const IVFittingSetup<size>& config)
	{
		using Settings = IVSimplexOptimizerSettings<M>;
		using Builder = typename Settings::IVSimplexOptimizerSettingsBuilder;

		LogCreatorSettings<size> logSettings{ config.simplexMin, config.simplexMax };
		logSettings.setP(config.logP);

		Builder builder{};
		builder.errorModel(Chi2ErrorModel{});
		builder.addLogCreatorSettings(logSettings);
		builder.addOperationSettings({ {BasicOperationsEnum::Reflect, config.reflec_coeff},
							   {BasicOperationsEnum::Expand, config.expand_coeff},
							   {BasicOperationsEnum::Contract, config.contract_coeff},
							   {BasicOperationsEnum::Shrink, config.shrink_coeff} });
		builder.minError(config.minError);
		builder.maxIteration(config.maxIteration);

		Settings settings = builder.build();

		IVSimplexOptimizer<M> optimizer{ settings };

		optimizer.setUp();

		return optimizer;
	}
	template <Model M, size_t size>
	NumericStorm::Fitting::Fitter<IVSimplexOptimizer<M>> getFitter(const IVFittingSetup<size>& config)
	{
		return Fitter<IVSimplexOptimizer<M>>{getOptimizer<M, size>(config)};
	}
}
