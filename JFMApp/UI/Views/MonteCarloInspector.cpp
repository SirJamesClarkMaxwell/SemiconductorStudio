#include "pch.hpp"

#include "Widgets.hpp"

namespace JFMApp::Views {

	std::vector<std::array<double, 3>> chi2vec = {
		{1.0, 4.0, 9.0},
		{2.30, 6.18, 11.8},
		{3.53, 8.02, 14.2},
		{4.72, 9.72, 16.3},
		{5.89, 11.3, 18.2},
		{7.04, 12.8, 20.1}
	};

	static int InputTextCallback(ImGuiInputTextCallbackData* data)
	{
		std::string* str = (std::string*)data->UserData;
		if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
		{
			str->resize(data->BufTextLen);
			data->Buf = (char*)str->c_str();
		}
		return 0;
	}

	static size_t getMCColor(double error, size_t dof) {
		size_t i = 0;
		dof = 4;
		auto& vec = chi2vec[dof - 1];

		if (error > vec[0])
			i = 1;
		if (error > vec[1])
			i = 2;

		return i;
	}

	static void drawPlot(Data::PlotData::MCPlotsData& mc, Data::NumericsConfig& nConf) {
		ImGui::ColorEdit4("Sigma 1", &mc.sig[0].x, ImGuiColorEditFlags_NoInputs);
		ImGui::SameLine();
		ImGui::ColorEdit4("Sigma 2", &mc.sig[1].x, ImGuiColorEditFlags_NoInputs);
		ImGui::SameLine();
		ImGui::ColorEdit4("Sigma 3", &mc.sig[2].x, ImGuiColorEditFlags_NoInputs);

		ImGui::SameLine(0.0f, 20.0f);

		std::string& prX = nConf.parameters[mc.parameters.first];

		const auto& params = nConf.modelParameters[mc.mc.modelID];
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.15f);
		if (ImGui::BeginCombo("X", prX.c_str())) {

			for (const auto& param : params) {
				if (param != mc.parameters.second)
					if (ImGui::Selectable(nConf.parameters[param].c_str(), param == mc.parameters.first))
						mc.parameters.first = param;
			}

			ImGui::EndCombo();
		}

		ImGui::SameLine();

		std::string& prY = nConf.parameters[mc.parameters.second];

		if (ImGui::BeginCombo("Y", prY.c_str())) {

			for (auto& param : params) {
				if (param != mc.parameters.first)
					if (ImGui::Selectable(nConf.parameters[param].c_str(), param == mc.parameters.second))
						mc.parameters.second = param;
			}

			ImGui::EndCombo();
		}

		ImGui::SameLine();
		if (ImGui::Button("Save") && mc.save)
			mc.save();


		ImGui::SameLine();
		ImGui::Value("Iterations: ", (int)mc.mc.iterations);
		ImGui::SameLine();
		ImGui::Text("Noise: %e", (float) mc.mc.sigma);

		ImGui::PopItemWidth();

		ImVec2 plotAreaSize = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
		if (ImPlot::BeginPlot(mc.name.c_str(), plotAreaSize, Data::PlotData::plotSettings.flags)) {

			ImPlot::SetupAxes(prX.c_str(), prY.c_str(), Data::PlotData::plotSettings.xFlags, Data::PlotData::plotSettings.yFlags);
			
			for (auto& d : mc.mc.data) {
				ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, -1, mc.sig[getMCColor(d.error, mc.mc.fixConfig.size())], -1.0f, mc.sig[getMCColor(d.error, mc.mc.fixConfig.size())]);

				ImPlot::PlotScatter("MC", &d.parameters[mc.parameters.first], &d.parameters[mc.parameters.second], 1);
			}

			ImPlot::EndPlot();
		}

	}


	void Widgets::MonteCarloInspector(Data::PlotData& data) {
		if (!data.paramConfig) return;
		auto& nConf = *(data.paramConfig);


		ImGuiTabBarFlags tabFlags = ImGuiTabBarFlags_NoTooltip
			| ImGuiTabBarFlags_FittingPolicyScroll;

		if (ImGui::BeginTabBar("MC Tabs", tabFlags)) {


			if (ImGui::TabItemButton("Add", ImGuiTabItemFlags_Trailing | ImGuiTabItemFlags_NoTooltip)) {
				data.mcTabs.push_back(data.mcTabs.back() + 1);
				std::string id = "MC Tab Dock" + std::to_string(data.mcTabs.back());
				data.tabsIDs.push_back(ImGui::GetID(id.c_str()));
			}
			int activeTab = data.mcTabs[0];
			for (auto& tab : data.mcTabs) {
				std::string name = "MC group " + std::to_string(tab);
				if (ImGui::BeginTabItem(name.c_str(), nullptr, ImGuiTabItemFlags_None))
				{
					activeTab = tab;
					ImVec2 s = ImVec2(ImGui::GetContentRegionAvail().x * 0.75f, ImGui::GetContentRegionAvail().y);
					ImGuiChildFlags cf = ImGuiChildFlags_Border;
					ImGui::BeginChild("PlotsArea", s, cf);

					{
						//auto& name = data.mcTempName;
						auto& name = data.active->name;
						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
						ImGui::InputTextWithHint("##Name", "Name",
							(char*)name.c_str(),
							name.capacity() + 1,
							ImGuiInputTextFlags_CallbackResize,
							InputTextCallback,
							(void*)&name);
						data.mcTempName = data.active->name;
						ImGui::SameLine();

						auto& tempParams = data.mcTempParams;

						if (data.activeMC) {
							auto& mc = *(data.activeMC);

							std::string& prX = nConf.parameters[tempParams.first];


							auto& params = nConf.modelParameters[mc.modelID];

							if (ImGui::BeginCombo("X", prX.c_str())) {

								for (auto& param : params) {
									if (param != tempParams.second)
										if (ImGui::Selectable(nConf.parameters[param].c_str(), param == data.mcTempParams.first))
											data.mcTempParams.first = param;
								}

								ImGui::EndCombo();
							}

							ImGui::SameLine();

							std::string& prY = nConf.parameters[tempParams.second];

							if (ImGui::BeginCombo("Y", prY.c_str())) {

								for (auto& param : params) {
									if (param != tempParams.first)
										if (ImGui::Selectable(nConf.parameters[param].c_str(), param == data.mcTempParams.second))
											data.mcTempParams.second = param;
								}

								ImGui::EndCombo();
							}
						}

						ImGui::SameLine();

						bool cond = data.activeMC;

						auto str = std::find_if(data.mcPlots.begin(), data.mcPlots.end(), [&](const auto& mc) {
							return mc.name == data.mcTempName;
							});

						//if (str != data.mcPlots.end() || data.mcTempName.size() == 0) cond = false;

						if (!cond)
							ImGui::BeginDisabled();

						if (ImGui::Button("Add plot")) {
							Data::PlotData::MCPlotsData mcData{};

							mcData.name = data.mcTempName;
							mcData.parameters = data.mcTempParams;
							mcData.mc = *data.activeMC;
							mcData.tab = tab;

							

							data.mcPlots.push_back(mcData);
							data.mcPlots.back().save = [&data]() {
								data.m_saveMCPlot(data.mcPlots.size() - 1);
								};

						}
						ImGui::SameLine();
						if (ImGui::Button("Save Uncertainty")) {
							data.m_saveMCUncertainty();
						}

						if (!cond)
							ImGui::EndDisabled();


					}
					ImGui::PopItemWidth();
					//dockspace
					{

						ImGuiChildFlags window_flags = ImGuiChildFlags_Border;

						ImVec2 child_size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
						std::string dName = "##tabDock" + std::to_string(tab);
						ImGui::BeginChild(dName.c_str(), child_size);

						ImGuiID dockspace_id = data.tabsIDs[tab - 1];
						ImGui::DockSpace(dockspace_id, child_size, ImGuiDockNodeFlags_None);

						ImGui::EndChild();
					}

					ImGui::EndChild();



					ImGui::SameLine();

					{
						/*ImVec2 listSize = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);

						if (ImGui::BeginListBox("##List", listSize) && data.characteristics) {
							


								ImGui::Text(ch.name.c_str());
								ImGui::Separator();

								for (auto& mc : ch.mcData) {
									if (ImGui::Selectable(mc.sim_name.c_str(), data.activeMC == &mc)) {
										data.activeMC = &mc;

										auto& tempParams = data.mcTempParams;

										tempParams.first = nConf.modelParameters[ch.modelID][0];
										tempParams.second = nConf.modelParameters[ch.modelID][1];
									}
									ImGui::Text("Noise: ");
									ImGui::SameLine();
									ImGui::Text(std::to_string(mc.sigma).c_str());
									ImGui::Separator();
									for (auto& [key, value] : mc.fixConfig) {
										ImGui::Text(nConf.parameters[key].c_str());
										ImGui::SameLine();
										ImGui::Text(": ");
										ImGui::SameLine();
										ImGui::Text(std::to_string(value).c_str());
									}

								}

								


							
							}
							ImGui::EndListBox();*/
						ImVec2 cSize = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
						if (ImGui::BeginChild("MC Table", cSize, ImGuiChildFlags_Border)) {
							ImGuiTableFlags taflags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;
							ImVec2 tsize = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
							if (ImGui::BeginTable("MC List", 4, taflags)) {
								ImGui::TableSetupColumn("Item", ImGuiTableColumnFlags_NoHide);
								ImGui::TableSetupColumn("Fix Config", ImGuiTableColumnFlags_WidthFixed, 50.0f);
								ImGui::TableSetupColumn("Noise", ImGuiTableColumnFlags_WidthFixed, 50.0f);
								ImGui::TableSetupColumn("Iterations", ImGuiTableColumnFlags_WidthFixed, 50.0f);
								ImGui::TableHeadersRow();
								for (auto& ch : *(data.characteristics)) {
									if (!ch.checked) continue;
									std::scoped_lock lk{ *ch.mcMutex };
									ImGui::TableNextRow();
									ImGui::TableNextColumn();

									bool open = ImGui::TreeNode(ch.name.c_str());
									ImGui::TableNextColumn();
									ImGui::Text("--");
									ImGui::TableNextColumn();
									ImGui::Text("--");
									ImGui::TableNextColumn();
									ImGui::Text("--");
									if (open) {

										for (auto& mc : ch.mcData) {
											ImGui::TableNextRow();
											ImGui::TableNextColumn();
											if (ImGui::Selectable(mc.sim_name.c_str(), data.activeMC == &mc)) {
												data.activeMC = &mc;

												auto& tempParams = data.mcTempParams;
												data.active = &ch;
												tempParams.first = nConf.modelParameters[ch.modelID][0];
												tempParams.second = nConf.modelParameters[ch.modelID][1];
											}
											ImGui::TableNextColumn();
											for (auto& [key, value] : mc.fixConfig) {
												ImGui::Text(nConf.parameters[key].c_str());
												ImGui::SameLine();
												ImGui::Text(": ");
												ImGui::SameLine();
												ImGui::Text(std::to_string(value).c_str());
											}
											ImGui::TableNextColumn();
											ImGui::Text(std::to_string(mc.sigma).c_str());
											ImGui::TableNextColumn();
											ImGui::Text(std::to_string(mc.iterations).c_str());
										}

										ImGui::TreePop();
									}

								}
							}
							ImGui::EndTable();
						}
						ImGui::EndChild();
					}

					ImGui::EndTabItem();
				}


			}


			{
				for (auto tab : data.mcTabs) {
					if (tab == activeTab) continue;
					ImGuiChildFlags child_flags = ImGuiChildFlags_Border;
					ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse;

					ImVec2 child_size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
					std::string dName = "##tabDockHidden" + std::to_string(tab);
					//ImGui::BeginChild(dName.c_str(), child_size, child_flags, window_flags);

					ImGuiID dockspace_id = data.tabsIDs[tab - 1];
					ImGui::DockSpace(dockspace_id, child_size, ImGuiDockNodeFlags_KeepAliveOnly);

					//ImGui::EndChild();
				}
			}

			{
				for (auto& mc : data.mcPlots) {


					ImGui::SetNextWindowDockID(data.tabsIDs[mc.tab - 1], ImGuiCond_Once);
					ImGui::Begin(mc.name.c_str(), nullptr, ImGuiWindowFlags_NoCollapse);
					drawPlot(mc, nConf);
					ImGui::End();

				}
			}


			ImGui::EndTabBar();
		}
	}
};