#pragma once
#include "stl.hpp"

namespace UI::Data {

	struct FittingOptions {
		bool fix_rch = false;
		bool fix_rs = false;
		bool use_bounds = false;
		bool annealing_pushed = false;
		int depth = 1;
		int selected_model = 0;
		std::vector<std::string> list{};
	};

	struct ChisqLists {
		int iterations = 0;
		double best_chi = 0.0;
		std::string prev_op{};
		std::string next_op{};
	};

	struct VertexPlotData {
		std::vector<double> comp_values{};
		int vertex_num{};
		double chisq{};
		double ann_chisq{};
		std::vector<double> parameters{};
	};

	struct PlotSetData {
		std::vector<double> arguments{};
		std::vector<double> given_values{};
		bool lin_x_scale = true;
		bool lin_y_scale = false;
		std::pair<std::string, std::string> axis{};
		std::vector<VertexPlotData> plots;
		std::vector<std::string> parameter_names{};
	};

	struct UIOptions {
		const std::string doub_disp_f{ "%.9f" };
		//add other options(almost no constants in the code)
	};

	struct ControlButtons {
		bool step_pressed = false;
		bool step10_pressed = false;
		bool reset_pressed = false;
		bool ldata_pressed = false;
	};

	class FittingTesterData {
		FittingOptions m_fit_test_options{};
		ChisqLists m_chisq_data{};
		PlotSetData m_plots{};
		ControlButtons m_buttonsData{};
		UIOptions m_ui_opts{};

	public:

		FittingOptions* getFittingOptions() { return &m_fit_test_options;  }
		ChisqLists* getChisqData() { return &m_chisq_data; }
		PlotSetData* getPlots() { return &m_plots; }
		ControlButtons* getButtonsData() { return &m_buttonsData; }
		UIOptions* getUIOpts() { return &m_ui_opts; }
	};
}