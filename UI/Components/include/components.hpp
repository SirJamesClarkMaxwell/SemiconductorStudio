#pragma once
#include "data.hpp"
#include "stl.hpp"
#include "imgui.h"
#include "implot.h"

namespace UI::Components {
	void parameter_input_double(const std::string& pName, double* val);
	void show_button(bool* pressed);

	void draw_bg_window();

	void draw_fitting_options(UI::Data::FittingTesterData* state);
	void draw_chisq_lists(UI::Data::FittingTesterData* state);
	void draw_vertex_plot(UI::Data::FittingTesterData* state, int plot_num);
	void draw_plot_grid(UI::Data::FittingTesterData* state);
	void draw_control_buttons(UI::Data::FittingTesterData* state);

	void window_wrap(const std::string& name, std::function<void()> content);

}
