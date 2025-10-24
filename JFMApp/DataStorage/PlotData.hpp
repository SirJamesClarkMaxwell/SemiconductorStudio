#pragma once
#include "pch.hpp"
#include "Characteristic.hpp"
#include "implot.h"

namespace JFMApp::Data
{
	struct PlotData
	{

		void submitMC(const MCOutput &mcData, Characteristic &ch);

		// references to the selected characteristics
		// plot options
		//--scale
		//--color
		//  --ranged
		//  --fitted
		//  --main
		// pointer to active
		// temporary parameters
		// parameters history
		// callbacks
		// hide/show non-active

		std::vector<Characteristic> *characteristics{nullptr};

		bool logX{false};
		bool logY{false};

		bool plotRanged{false};
		bool plotFitted{false};
		bool plotOriginal{true};
		bool plotTunned{false};

		ImVec4 colorRanged{0.61f, 0.26f, 0.96f, 1.0f}, colorFitted{0.0f, 1.0f, 0.0f, 1.0f}, colorTunned{1, 1, 0, 1};

		bool hideNonActive{false};

		Characteristic *active{nullptr};
		NumericsConfig *paramConfig{nullptr};

		struct MCPlotsData
		{
			Characteristic::MCSimulation mc{};
			std::array<ImVec4, 3> sig{ImVec4{0.0f, 1.0f, 0.0f, 1.0f}, ImVec4{1.0f, 1.0f, 0.0f, 1.0f}, ImVec4{1.0f, 0.0f, 0.0f, 1.0f}};
			std::pair<ParameterID, ParameterID> parameters{};
			std::string name{};
			int tab{-1};
			int id;
			std::function<void(int id)> save{};
			std::function<void(int id)> remove{};
		};

		std::vector<MCPlotsData> mcPlots{};

		std::string mcTempName{};
		std::pair<ParameterID, ParameterID> mcTempParams{};

		ImVector<unsigned int> mcTabs{};
		ImVector<ImGuiID> tabsIDs{};

		Characteristic::MCSimulation *activeMC{nullptr};
		int mcCount{-1};

		bool configAll{false};

		// global config for all characteristics
		ModelID globalModelID{};
		std::unordered_map<ParameterID, bool> globalFixedParameterIDs{};
		Characteristic::MCConfig globalMCConfig{};

		// global saved config for all characteristics
		ModelID savedGlobalModelID{};
		std::unordered_map<ParameterID, bool> savedGlobalFixedParameterIDs{};
		Characteristic::MCConfig savedGlobalMCConfig{};

		std::function<void(std::vector<JFMApp::Data::Characteristic> &characteristics)> m_saveParametersCallback{};
		std::filesystem::path currentPath;

		std::function<void()> m_estimateCallback{};
		std::function<void()> m_fitCallback{};
		std::function<void()> m_tuneCallback{};

		std::function<void()> m_saveMCConfCallback{};
		std::function<void()> m_performMCCallback{};
		std::function<void()> m_performMCOnAllCallback{};

		std::function<void()> m_saveMCUncertainty{};
		std::function<void(int size)> m_saveMCPlot{};
		std::function<void()> m_saveMCData{};
		std::function<void()> m_plotAllMC{};
		std::function<void()> m_saveAllMCPlots{};
		std::function<void()> m_clearAllPlots{};
		void saveOneSimulation(const std::filesystem::path &path, const JFMApp::Data::Characteristic &characteristic);
		struct PlotSettings
		{
			ImPlotFlags flags = ImPlotFlags_NoLegend;
			ImPlotAxisFlags xFlags = ImPlotAxisFlags_None;
			ImPlotAxisFlags yFlags = ImPlotAxisFlags_None;
		};
		enum PlotType
		{
			Fit = 0,
			Tunned
		};
		using ArrheniusFunctionType = std::function<double(const ParameterMap& pMap)>;
		static PlotSettings plotSettings;
		enum ArrheniusPlotType
		{
			A_ln_I0 = 0,
			A,
			I0,
			Rs,
			Rsh,
			Rsh2,
			alpha,
			// Isc,
			None,
		};
		struct ArrheniusPlotSetting
		{
			ArrheniusPlotSetting();
			bool inversionT{true};
			bool fitPotArea{false};
			ImVec4 tunedColor{1, 1, 0, 1};
			ImVec4 fittedColor{0, 1, 0, 1};
			std::vector<float> Temperature;
			ArrheniusPlotType currentPlotType;
			static inline std::string convertTypeToString(ArrheniusPlotType type) 
			{
				switch (type)
				{
				case ArrheniusPlotType::A_ln_I0:
					return "A_ln_I0";
				case ArrheniusPlotType::I0:
					return "I0";
				case ArrheniusPlotType::A:
					return "A";
				case ArrheniusPlotType::Rs:
					return "Rs";
				case ArrheniusPlotType::Rsh:
					return "Rsh";
				case ArrheniusPlotType::Rsh2:
					return "Rsh2";
				case ArrheniusPlotType::alpha:
					return "alpha";
				default:
					return "None";
				}
			};;
			static inline ArrheniusPlotType convertStringToType(std::string type) {
				if ("A_ln_I0")
					return ArrheniusPlotType::A_ln_I0;
				if ("A")
					return ArrheniusPlotType::A;
				if ("I0")
					return ArrheniusPlotType::I0;
				if ("Rs")
					return ArrheniusPlotType::Rs;
				if ("Rsh")
					return ArrheniusPlotType::Rsh;
				if ("Rsh2")
					return ArrheniusPlotType::Rsh2;
				if ("alpha")
					return ArrheniusPlotType::alpha;

				return ArrheniusPlotType::None;
			};
			void updateTemperatures(const std::vector<Characteristic> &characteristics);
			static std::unordered_map<ArrheniusPlotType, ArrheniusFunctionType> functions;
		};

		struct ArrheniusPlotData
		{

			ArrheniusPlotType arrheniusPlotType;
			std::array<std::vector<float>, 2> yData;
			void updateStoredData(const std::vector<Characteristic> &characteristics);
			ArrheniusFunctionType appliedFunction = nullptr;
			void Plot(const std::string& name,const PlotData::ArrheniusPlotSetting& settings,ImGuiID dockID);
			
		};
		
		ArrheniusPlotData AddPlot(ArrheniusPlotType type);
		ArrheniusPlotSetting m_arrheniusPlotSettings;
		std::vector<ArrheniusPlotData> m_arrheniusPlotData;
	};
}
