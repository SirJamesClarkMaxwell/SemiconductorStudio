#pragma once

#include "pch.hpp"

#include "NumericsConfig.hpp"
#include "IDataManager.hpp"
#include "imgui.h"

#include "implot.h"

namespace JFMApp::Data {
	using namespace JFMService::DataManagementService;
	

	struct Characteristic
	{

		Characteristic() {
			initState();
		}
		Characteristic(const Characteristic&) = default;
		Characteristic(Characteristic&&) = default;
		Characteristic& operator=(const Characteristic&) = default;
		Characteristic& operator=(Characteristic&&) = default;
		~Characteristic() = default;

		Characteristic(const CharacteristicData& loaded);

		EstimateInput getEstimateInput();
		FittingInput getFittingInput();
		void submitFitting(const ParameterMap& parameters, double error);
		CalculatingData getCalculatingData();
		CalculatingData getTuningData();
		MCInput getMCConfig();
		void submitMC(const MCOutput& mcData);

		void initState();

		//containers of V, I, J
		//temperature
		//name
		//description
		//montecarlo sub characteristic
		//fitted characteristic
		//validity of subcharacteristics
		//data range

		//fitting configuration
		//--model
		//--fix
		//--initial guess
		//--mc
		//--notes

		std::vector<double> V{};
		std::vector<double> I{};
		std::vector<double> J{};

		double T{};

		//instead of ECS
		std::filesystem::path path{};
		NumericsConfig nConfig{};

		std::pair<size_t, size_t> dataRange{0, 1};
		bool isFitted{ false };
		bool isSimulated{ false };
		bool checked{ false };
		Data::ModelID forcedModelID{0};

		ImVec4 color{ 0.96f, 0.53f, 0.26f, 1.0f };
		float weight{ 1.0 };

		std::string name{};
		std::string description{};

		//montecarlo
		struct MCData {
			MCData() = default;
			MCData(const MCData&) = default;
			MCData(MCData&&) = default;
			MCData& operator=(const MCData&) = default;
			MCData& operator=(MCData&&) = default;
			~MCData() = default;


			MCData(const MCResult& res) : parameters{ res.foundParameters }, error{ res.error } {}
			MCData& operator=(const MCResult& res) {
				parameters = res.foundParameters;
				error = res.error;
				return *this;
			}


			ParameterMap parameters{};
			double error{ -1.0 };
		};

		struct MCSimulation {
			std::vector<MCData> data{};
			std::string sim_name{};
			ParameterMap fixConfig{};
			size_t iterations{ 0 };
			double sigma{ 1.0 };
			ModelID modelID{};
			ParameterMap trueParameters{};
			std::filesystem::path relPath{};
		};

		std::vector<MCSimulation> mcData{};

		

		//fitted
		ParameterMap fittedParameters{};
		std::vector<double> fittedI{};
		bool fitted{0};
		double fitError{ -1.0 };



		//tuned
		std::vector<double> tunedI{};
		ParameterMap tunedParameters{};
		std::unordered_map<ParameterID, bool> tempParametersActive{};

		double tuneError{ -1.0 };

		std::function<void()> m_tuneCallback{};



		//fitting configuration
		ModelID modelID{ 3};
		double ShortCircuitCurrent{-1.0};
		CharacteristicType characteristicType = 0;
		std::unordered_map<ParameterID, bool> fixedParameterIDs{};
		ParameterMap fixedParametersValues{};
		ParameterMap initialGuess{};

		struct MCConfig {
			size_t n{};
			size_t saveN{};
			double sigma{ 1.0 };

			std::partial_ordering operator<=>(const MCConfig&) const = default;

		} mcConfig{};
		ParamBounds bounds{};

		bool useBounds{ false }, useInitial{ false };

		std::string notes{};


		//last saved config
		ModelID savedModelID{};
		std::unordered_map<ParameterID, bool> savedFixedParameterIDs{};
		ParameterMap savedFixedParametersValues{};
		ParameterMap savedInitialGuess{};
		MCConfig savedMCConfig{};
		ParamBounds savedBounds{};

		bool savedUseBounds{ false }, savedUseInitial{ false };

		std::string savedNotes{};

		std::shared_ptr<std::mutex> mcMutex{ nullptr };


		static double TFL(double v, void*) {
			return std::log(std::max(v, std::numeric_limits<double>::epsilon()));
		}

		static double TFNL(double v, void*) {
			return std::exp(v);
		}
	};

	

};


