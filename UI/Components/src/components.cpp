#pragma once
#include "components.hpp"

namespace UI::Components {
	
	

	void draw_bg_window() {
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		window_flags |= ImGuiWindowFlags_NoScrollbar;
		window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoCollapse;
		window_flags |= ImGuiWindowFlags_NoNav;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;


		const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y));
		ImGui::SetNextWindowSize(ImVec2(main_viewport->WorkSize.x, main_viewport->WorkSize.y));
		bool open = true;
		ImGui::Begin("##", &open, window_flags);

		ImGui::End();
	}

	void draw_fitting_options(UI::Data::FittingTesterData* state) {
		auto* options = state->getFittingOptions();
		ImGui::Spacing();
		const char* preview = (options->selected_model >= 0 && options->selected_model < options->list.size()) ?
			options->list[options->selected_model].c_str() :
			"";

		ImGui::PushItemWidth(200);
		ImGui::BeginGroup();
		

		if (ImGui::BeginCombo("Fitted model", preview))
		{
			for (int n = 0; n < options->list.size(); n++)
			{
				const bool is_selected = (options->selected_model == n);
				if (ImGui::Selectable(options->list[n].c_str(), is_selected))
					options->selected_model = n;

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		ImGui::InputInt("Depth", &(options->depth)); 
		ImGui::EndGroup();

		ImGui::SameLine(0, ImGui::GetStyle().ItemSpacing.x * 2);

		ImGui::BeginGroup();
		options->annealing_pushed = ImGui::Button("Simulated Annealing"); 
		ImGui::Checkbox("Use bounds", &(options->use_bounds));
		ImGui::EndGroup();

		ImGui::SameLine(0, ImGui::GetStyle().ItemSpacing.x * 2);

		ImGui::BeginGroup();
		ImGui::Checkbox("Fix Rch", &(options->fix_rch));
		ImGui::Checkbox("Fix Rs", &(options->fix_rs));
		ImGui::EndGroup();

		ImGui::PopItemWidth();
	}

	void draw_chisq_lists(UI::Data::FittingTesterData* state) {
		auto* data = state->getChisqData();
		auto* chisq_data = state->getPlots();
		const std::string& format = state->getUIOpts()->doub_disp_f;

		ImGui::BeginGroup();
		ImGui::Spacing();
		ImGui::Text("Number of iterations: %d", data->iterations); 

		ImGui::Text(std::string{ "Best Chi^2: " }.append(format).c_str(), data->best_chi);

		ImGui::EndGroup();

		

		ImGui::BeginGroup();
		ImGui::Spacing();
		ImGui::Text("Previous operation: %s", data->prev_op.c_str());
		ImGui::Text("Next operation: %s", data->next_op.c_str());

		ImGui::EndGroup();

		

		ImGui::BeginGroup();
		ImGui::Spacing();
		static ImGuiTableFlags flags = ImGuiTableFlags_BordersOuter
									| ImGuiTableFlags_BordersInnerV
									| ImGuiTableFlags_RowBg
									| ImGuiTableFlags_NoHostExtendX
									| ImGuiTableFlags_SizingFixedFit
									| ImGuiTableFlags_ScrollY;

		ImVec2 outer_size = ImVec2(200.0f, ImGui::GetTextLineHeightWithSpacing() * 7);
		ImGui::Spacing();
		ImGui::Text("Simplex vertecies");

		if (ImGui::BeginTable("simpl_chi", 2, flags, outer_size)) {

			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableSetupColumn("Vertex");
			ImGui::TableSetupColumn("Chi^2");
			ImGui::TableHeadersRow();

			for (auto& plot : chisq_data->plots) {
				ImGui::TableNextRow();

				ImGui::TableNextColumn();
				ImGui::Text("%d", plot.vertex_num);


				ImGui::TableNextColumn();
				ImGui::Text(format.c_str(), plot.chisq);
			}

			ImGui::EndTable();
		}

		ImGui::EndGroup();


		

		ImGui::BeginGroup();
		ImGui::Spacing();
		
		ImGui::Spacing();
		ImGui::Text("Annealing vertecies");

		if (ImGui::BeginTable("ann_simpl_chi", 2, flags, outer_size)) {

			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableSetupColumn("Vertex");
			ImGui::TableSetupColumn("Chi^2");
			ImGui::TableHeadersRow();

			for (auto& plot : chisq_data->plots) {
				ImGui::TableNextRow();

				ImGui::TableNextColumn();
				ImGui::Text("%d", plot.vertex_num);


				ImGui::TableNextColumn();
				ImGui::Text(format.c_str(), plot.ann_chisq);
			}

			ImGui::EndTable();
		}

		ImGui::EndGroup();

	}

	static inline double TR_FW_LN(double v, void*) {
		return log(v);
	}

	static inline double TR_INV_LN(double v, void*) {
		return exp(v);
	}

	void draw_vertex_plot(UI::Data::FittingTesterData* state, int plot_num) {
		auto* data = state->getPlots();
		const std::string& format = state->getUIOpts()->doub_disp_f;

		if (data->arguments.size() != data->given_values.size()) return;

		ImGui::BeginGroup();
		ImGui::Spacing();

		std::string name_base = "Vertex ";

		const double* xs = data->arguments.data();
		const double* mys = data->given_values.data();
		const double* fys = data->plots[plot_num].comp_values.data();



		ImVec2 plot_pos = ImGui::GetCursorScreenPos();

		if (ImPlot::BeginPlot((name_base.append(std::to_string(data->plots[plot_num].vertex_num)).c_str()))) {
			ImPlot::SetupAxes(data->axis.second.c_str(), data->axis.first.c_str());
			if (!data->lin_x_scale) ImPlot::SetupAxisScale(ImAxis_X1, TR_FW_LN, TR_INV_LN);
			if (!data->lin_y_scale) ImPlot::SetupAxisScale(ImAxis_Y1, TR_FW_LN, TR_INV_LN);


			if (data->given_values.size() != 0)
				ImPlot::PlotLine("Measurement", xs, mys, data->arguments.size());
			if (data->plots[plot_num].comp_values.size() != 0)
				ImPlot::PlotLine("Fitted Curve", xs, fys, data->plots[plot_num].comp_values.size());

			ImPlot::EndPlot();
		}

		if (data->parameter_names.size() != 0 && (data->parameter_names.size() == data->plots[plot_num].parameters.size())) {
			std::string id = "##ver_params";

			ImVec2 child_size(150, 0);
			ImVec2 plot_size = ImGui::GetItemRectSize();
			ImVec2 child_pos(plot_pos.x + plot_size.x - child_size.x * 1.2f,
				plot_pos.y + plot_size.y * 0.13f);

			ImGuiChildFlags child_flags = ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY;

			ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(15, 15, 15, 255));

			ImGui::SetCursorScreenPos(child_pos);

			ImGui::BeginChild(id.append(std::to_string(plot_num)).c_str(), child_size, child_flags);
			
			std::string param{};
			for (int p = 0; p < data->parameter_names.size(); p++) {
				param = data->parameter_names[p];
				param.append(": ").append(format);
				ImGui::Text(param.c_str(), data->plots[plot_num].parameters[p]);
			}

			ImGui::EndChild();

			ImGui::PopStyleColor();
		}

		ImGui::EndGroup(); 
	}

	

	void draw_plot_grid(UI::Data::FittingTesterData* state) {
		auto* data = state->getPlots();
		static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY;

		if (ImGui::BeginTable("Plots", 2, flags))
		{
			for (int plot_num = 0; plot_num < data->plots.size(); plot_num++)
			{
				ImGui::TableNextColumn();
				draw_vertex_plot(state, plot_num);
				ImGui::Spacing();
			}
			ImGui::EndTable();
		}
	}

	void draw_control_buttons(UI::Data::FittingTesterData* state) {
		auto* data = state->getButtonsData();
		ImGui::BeginGroup();
		
		float height = ImGui::GetItemRectSize().y - ImGui::GetStyle().ItemSpacing.y;
		ImVec2 b_size(100, height);
		data->step_pressed = ImGui::Button("Step", b_size);

		ImGui::SameLine();

		data->step10_pressed = ImGui::Button("Step 10", b_size);

		ImGui::SameLine();

		data->reset_pressed = ImGui::Button("Reset", b_size);

		ImGui::SameLine();

		data->ldata_pressed = ImGui::Button("Load Data", b_size);

		ImGui::SameLine();


		ImGui::EndGroup();
	}

	void draw_fitting_tester(std::shared_ptr<Data::FittingTesterData> state) {
		ImGui::Begin("Fitting Tester");


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


		ImGui::End();
	}



	
}