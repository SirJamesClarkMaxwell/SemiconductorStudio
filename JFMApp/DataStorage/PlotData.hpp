#pragma once
#include "pch.hpp"
#include "Characteristic.hpp"

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
		bool plotTunned{ false };

		ImVec4 colorRanged{ 0.61f, 0.26f, 0.96f, 1.0f }, colorFitted{ 0.0f, 1.0f, 0.0f, 1.0f }, colorTunned{1,1,0,1};

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
		};

		std::vector<MCPlotsData> mcPlots{};

		std::string mcTempName{};
		std::pair<ParameterID, ParameterID> mcTempParams{};

		ImVector<unsigned int> mcTabs{};
		ImVector<ImGuiID> tabsIDs{};

		Characteristic::MCSimulation *activeMC{nullptr};
		int mcCount{ -1 };

		bool configAll{false};

		// global config for all characteristics
		ModelID globalModelID{};
		std::unordered_map<ParameterID, bool> globalFixedParameterIDs{};
		Characteristic::MCConfig globalMCConfig{};

		// global saved config for all characteristics
		ModelID savedGlobalModelID{};
		std::unordered_map<ParameterID, bool> savedGlobalFixedParameterIDs{};
		Characteristic::MCConfig savedGlobalMCConfig{};

		std::function<void( std::vector<JFMApp::Data::Characteristic>& characteristics)> m_saveParametersCallback{};
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
		void saveOneSimulation(const std::filesystem::path& path, const JFMApp::Data::Characteristic& characteristic);
		struct PlotSettings
		{
			ImPlotFlags flags = ImPlotFlags_NoLegend;
			ImPlotAxisFlags xFlags = ImPlotAxisFlags_None;
			ImPlotAxisFlags yFlags = ImPlotAxisFlags_None;
		};

		static inline PlotSettings plotSettings;

	};
}
