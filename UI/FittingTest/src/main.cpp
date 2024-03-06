#include <iostream>
#include "tester.hpp"


void dr(std::shared_ptr<Data::FittingTesterData> state) {
	Components::draw_bg_window();


	auto content = [&state] {
		UI::Components::draw_fitting_options(state.get());
		ImGui::SameLine(0, 20);
		UI::Components::draw_control_buttons(state.get());

		static ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchProp
									| ImGuiTableFlags_BordersOuter
									| ImGuiTableFlags_BordersInnerV;


		if (ImGui::BeginTable("Layout", 2, flags)) {
			ImGui::TableNextColumn();
			UI::Components::draw_plot_grid(state.get());

			ImGui::TableNextColumn();
			UI::Components::draw_chisq_lists(state.get());


			ImGui::EndTable();
		}

		
	};

	Components::window_wrap("Fitting Tester", content);

	//subplots
	//	sizing
	//	data sharing
	//plots
	//	line plots
	//	scatter plots
	//axes
	//	log scale
	//	axis constraints
	//	infinite lines
	//	drag lines


	//auto range the voltage - simplex(prefit)
	//optional - possible to fix parameters in place or cange the bounds found in the first step
	//fit with annealing
	//mc sim


	


	ImGui::ShowDemoWindow();
	
	ImPlot::ShowDemoWindow();

}

int main() {

	Platform::Window::initProperties(1920, 1080, "FittingTester");


	Tester t1{};

	t1.run(dr);

	return 0;
}