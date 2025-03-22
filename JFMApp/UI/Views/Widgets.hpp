#pragma once
#include "pch.hpp"

#include "imgui.h"
#include "implot.h"

#include "ViewData.hpp"


namespace JFMApp::Views {

	struct Widgets {
		static void PlottingArea(Data::PlotData& data);
		static void BrowserArea(Data::BrowserData& data);
		static void CharacteristicList(Data::BrowserData& data);
		static void CharacteristicInspector(Data::PlotData& data);
		static void MonteCarloInspector(Data::PlotData& data);
		static void ArrheniusViewer(Data::PlotData& data);

		static void FileSelector(Data::BrowserData& data);
		static void ViewMenu(Data::UIState& data);
		static void DataGenerator(Data::BrowserData& data);

		static void global_ds();

		static inline ImGuiID mDS;
	};
}