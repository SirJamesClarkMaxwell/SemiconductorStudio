#include "pch.hpp"

#include "Widgets.hpp"

namespace JFMApp::Views {

	static std::vector<double> GeneratePowersOf10(double lower_bound, double upper_bound) {
		std::vector<double> powers_of_10;

		if (lower_bound > upper_bound) {
			std::swap(lower_bound, upper_bound);
		}

		int start_exponent = std::ceil(std::log10(lower_bound)) - 1;
		int end_exponent = std::floor(std::log10(upper_bound));

		for (int i = start_exponent; i <= end_exponent; ++i) {
			double base = std::pow(10, i);
			for (int j = 1; j <= 9; ++j) {
				double value = j * base;
				if (value >= lower_bound && value <= upper_bound) {
					powers_of_10.push_back(value);
				}
			}
		}

		return powers_of_10;
	}

	void Widgets::PlottingArea(Data::PlotData& data) {
		if (!data.characteristics || !data.paramConfig) return;
		if (ImGui::Begin("Plot Area")) {
			//plot and list are in the table
			ImVec2 tableSize = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
			ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_SizingStretchProp;

			if (ImGui::BeginTable("PlottingArea", 2, tableFlags, tableSize)) {

				ImGui::TableSetupColumn("Plot", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("List", ImGuiTableColumnFlags_WidthFixed, 150.0f);
				ImGui::TableHeadersRow();


				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				static ImPlotRect limits{};
				//draw the plot area
				{
					ImVec2 plotAreaSize = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
					if (ImPlot::BeginPlot("Characteristics", plotAreaSize, data.plotSettings.flags)) {

						
						
						ImPlot::SetupAxes("V", "I", Data::PlotData::plotSettings.xFlags, Data::PlotData::plotSettings.yFlags);

						Data::PlotData::plotSettings.xFlags &= ~ImPlotAxisFlags_AutoFit;
						Data::PlotData::plotSettings.yFlags &= ~ImPlotAxisFlags_AutoFit;

						
						if (data.logY && limits.Y.Min > 0.0) {
							std::vector<double> y_ticks = GeneratePowersOf10(limits.Y.Min, limits.Y.Max);


							std::vector<const char*> y_labels;
							std::vector<std::string> label_storage;

							for (double value : y_ticks) {

								double log_val = std::log10(value);
								if (std::floor(log_val) == log_val) {
									int exponent = std::log10(value); 
									label_storage.push_back("1e" + std::to_string(exponent)); 
								}
								else {
									label_storage.push_back(" ");
								}
							}

							for (auto& str : label_storage) {
								y_labels.push_back(str.c_str());
							}


							ImPlot::SetupAxisTicks(ImAxis_Y1, y_ticks.data(), y_ticks.size(), y_labels.data());
						}

						if (data.logX && limits.X.Min > 0.0) {
							std::vector<double> x_ticks = GeneratePowersOf10(limits.X.Min, limits.X.Max);

							std::vector<const char*> x_labels;
							std::vector<std::string> label_storage;

							for (double value : x_ticks) {

								double log_val = std::log10(value);
								if (std::floor(log_val) == log_val) {
									int exponent = std::log10(value);
									label_storage.push_back("1e" + std::to_string(exponent));
								}
								else {
									label_storage.push_back(" ");
								}
							}

							for (auto& str : label_storage) {
								x_labels.push_back(str.c_str());
							}



							ImPlot::SetupAxisTicks(ImAxis_X1, x_ticks.data(), x_ticks.size(), x_labels.data());
						}

						

						if (data.logX)
							ImPlot::SetupAxisScale(ImAxis_X1, Data::Characteristic::TFL, Data::Characteristic::TFNL);

						if (data.logY)
							ImPlot::SetupAxisScale(ImAxis_Y1, Data::Characteristic::TFL, Data::Characteristic::TFNL);

						limits = ImPlot::GetPlotLimits();


						if (data.active && data.hideNonActive) {
							auto& act = *(data.active);
							//main characteristics
							if (data.plotOriginal) {
								ImPlot::SetNextLineStyle(act.color, act.weight);
								ImPlot::PlotLine("Active", act.V.data(), act.I.data(), std::min(act.V.size(), act.I.size()));
							}

							//ranged characteristics
							if (data.plotRanged) {
								ImPlot::SetNextLineStyle(data.colorRanged);
								ImPlot::PlotLine(act.name.c_str(), act.V.data() + act.dataRange.first, act.I.data() + act.dataRange.first, std::min(act.dataRange.second - act.dataRange.first, act.I.size()));
							}

							//fitted characteristics
							if (act.isFitted && data.plotFitted) {
								ImPlot::SetNextLineStyle(data.colorFitted,2.0);
								ImPlot::PlotLine(act.name.c_str(), act.V.data() + act.dataRange.first, act.fittedI.data(), act.fittedI.size());
							}

							//tuned characteristics
							if (data.plotTunned) {
								ImVec4 invColor{};

								invColor.x = 1 - data.colorFitted.x;
								invColor.y = 1 - data.colorFitted.y;
								invColor.z = 1 - data.colorFitted.z;
								invColor.w = 1;
								ImPlot::SetNextLineStyle(data.colorTunned, 2.0);
								ImPlot::PlotLine(act.name.c_str(), act.V.data() + act.dataRange.first, act.tunedI.data(), act.tunedI.size());
							}

						}
						else if (!data.hideNonActive && data.characteristics) {
							for (auto& ch : *(data.characteristics)) {
								if (!ch.checked) continue;

								//main characteristics
								if (data.plotOriginal) {
									ImPlot::SetNextLineStyle(ch.color, ch.weight);
									ImPlot::PlotLine(ch.name.c_str(), ch.V.data(), ch.I.data(), std::min(ch.V.size(), ch.I.size()));
								}
								//ranged characteristics
								if (data.plotRanged) {
									ImPlot::SetNextLineStyle(data.colorRanged);
									ImPlot::PlotLine(ch.name.c_str(), ch.V.data() + ch.dataRange.first, ch.I.data() + ch.dataRange.first, std::min(ch.dataRange.second - ch.dataRange.first, ch.I.size()));
								}

								//fitted characteristics
								if (ch.isFitted && data.plotFitted) {
									ImPlot::SetNextLineStyle(data.colorFitted,2.0);
									ImPlot::PlotLine(ch.name.c_str(), ch.V.data() + ch.dataRange.first, ch.fittedI.data(), ch.fittedI.size());
								}

								//tuned characteristics
								if (data.plotTunned) {
									ImVec4 invColor{};

									invColor.x = 1 - data.colorFitted.x;
									invColor.y = 1 - data.colorFitted.y;
									invColor.z = 1 - data.colorFitted.z;
									invColor.w = 1;
									ImPlot::SetNextLineStyle(data.colorTunned, 2.0);
									ImPlot::PlotLine(ch.name.c_str(), ch.V.data() + ch.dataRange.first, ch.tunedI.data(), ch.tunedI.size());
								}

							}
						}

						//plot infinite lines

						if (data.active && data.plotRanged) {
							auto& act = *(data.active);
							if (act.dataRange.first != act.dataRange.second) {
								ImPlot::SetNextLineStyle(data.colorRanged);
								ImPlot::PlotInfLines("Down", &act.V[act.dataRange.first], 1);
								ImPlot::SetNextLineStyle(data.colorRanged);
								ImPlot::PlotInfLines("Up", &act.V[act.dataRange.second], 1);
							}
						}

						ImPlot::EndPlot();
					}
				}

				ImGui::TableNextColumn();
				//draw the characteristic list
				{
					int i = 0;
					ImVec2 listSize = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
					if (ImGui::BeginListBox("##List", listSize)) {
						if (data.characteristics) {

							for (auto& ch : *(data.characteristics)) {
								if (!ch.checked) continue;
								ImGui::PushID(i++);
								if (ImGui::Selectable(ch.name.c_str(), data.active == &ch)) {
									data.active = &ch;
								}
								ImGui::PopID();


							}

						}
						ImGui::EndListBox();
					}
				}

				ImGui::EndTable();
			}
		}
		ImGui::End();

		if (ImGui::Begin("Characteristic Settings")) {
			{

				//plot settings
				{
					if (ImGui::Button("Fit plot area")) {
						Data::PlotData::plotSettings.xFlags |= ImPlotAxisFlags_AutoFit;
						Data::PlotData::plotSettings.yFlags |= ImPlotAxisFlags_AutoFit;
					}

					ImGui::SameLine(0.0, 20.0);

					ImGui::Checkbox("Log X", &data.logX);
					ImGui::SameLine();
					ImGui::Checkbox("Log Y", &data.logY);

					ImGui::SameLine(0.0, 20.0);
					ImGui::Checkbox("Plot Ranged", &data.plotRanged);
					ImGui::SameLine();
					ImGui::ColorEdit4("Ranged Color", &data.colorRanged.x, ImGuiColorEditFlags_NoInputs);

					ImGui::SameLine(0.0, 20.0);
					ImGui::Checkbox("Plot Fitted", &data.plotFitted);
					ImGui::SameLine();
					ImGui::ColorEdit4("Fitted Color", &data.colorFitted.x, ImGuiColorEditFlags_NoInputs);

					ImGui::Checkbox("Hide non-active", &data.hideNonActive);
					ImGui::SameLine(0.0, 20.0);
					ImGui::Checkbox("Plot Original", &data.plotOriginal);
					ImGui::SameLine(0.0, 20.0);
					ImGui::Checkbox("Plot Tunned", &data.plotTunned);
					ImGui::Separator();
					if(ImGui::Button("Save Parameters"))
						data.m_saveParametersCallback(*data.characteristics);
					ImGui::Separator();
				}

				ImGui::Separator();


				if (!data.active) {
					ImGui::BeginDisabled();
				}

				//in case data->active is a nullptr
				static Data::Characteristic chr{};
				auto& act = data.active ? *(data.active) : chr;

				float groupH{ 0 };

				ImVec2 child_size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);

				ImGui::BeginChild("CharSetChild", child_size, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY);

				//draw the characteristic settings
				{

					int lower = 0;
					int middle = (int)act.dataRange.first + 1;
					int upper = act.V.size() - 1;
					groupH = ImGui::GetCursorPosY();
					ImGui::BeginGroup();

					auto vStr = data.active ? std::to_string(act.V[act.dataRange.second]) : "0";

					float sliderW = ImGui::GetCursorPosX();

					if (ImGui::SliderInt("Up range", (int*)&(act.dataRange.second), middle, upper, vStr.c_str())) 
					{
						data.m_estimateCallback();
						data.m_fitCallback();
					}

					sliderW = ImGui::GetCursorPosX() - sliderW;

					ImGui::SameLine(0.0, 50.0);

					float buttonW = ImGui::GetCursorPosX();

					if (ImGui::Button("Estimate")) {
						data.m_estimateCallback();
					}

					buttonW = ImGui::GetCursorPosX() - buttonW;

					vStr = data.active ? std::to_string(act.V[act.dataRange.first]) : "0";

					ImGui::PushItemWidth(sliderW);

					middle = std::max(lower, (int)act.dataRange.second - 1);

					if (ImGui::SliderInt("Down range", (int*)&(act.dataRange.first), lower, middle, vStr.c_str())) 
					{
						data.m_estimateCallback();
						data.m_fitCallback();
					}

					ImGui::PopItemWidth();

					ImGui::SameLine(0.0, 50.0);
					ImGui::PushItemWidth(buttonW);

					if (ImGui::Button("Fit")) {
						data.m_fitCallback();
					}

					ImGui::PopItemWidth();


					groupH = ImGui::GetCursorPosY() - groupH;

					ImGui::Text("Tune Error: %e", act.tuneError >= 0.0 ? act.tuneError : 0.0);

					ImGui::SameLine(0.0f, 20.0f);

					if (ImGui::Button("Update characteristic")) {
						act.tunedParameters.clear();
						for (auto& [key, val] : act.fittedParameters) 
						{
							if (act.tempParametersActive[key]) 
								val = act.tunedParameters[key];
							act.tunedParameters[key] = val;
						} 
						act.tunedI = act.fittedI;
						act.m_tuneCallback();
					}
					
					if (ImGui::BeginCombo("##CurrentModel", data.paramConfig->models[act.modelID].c_str(), ImGuiComboFlags_WidthFitPreview)) {
						for (const auto& [id, name] : data.paramConfig->models) 
						{
							if (ImGui::Selectable(name.c_str(), id )) 
							{
								act.modelID = id;
								act.savedModelID = id;
								data.m_estimateCallback();
								data.m_fitCallback();
							}
						}

						ImGui::EndCombo();
					}
					ImGui::SameLine();
					ImGui::Checkbox("Is Fitted", &act.fitted);
					ImGui::EndGroup();
				}

				ImGui::SameLine(0.0, 30.0);

				//draw the parameter tables
				{
					ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders
						| ImGuiTableFlags_Resizable
						| ImGuiTableFlags_Reorderable
						| ImGuiTableFlags_Hideable
						| ImGuiTableFlags_SizingStretchSame;

					ImVec2 tableSize = ImVec2(ImGui::GetContentRegionAvail().x, 0);

					if (ImGui::BeginTable("Parameters", 2, tableFlags, tableSize)) {

						ImGui::TableSetupColumn("Fitted");
						ImGui::TableSetupColumn("Tuned");

						ImGui::TableHeadersRow();

						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						if (ImGui::BeginTable("Fitted", 2, tableFlags & ~ImGuiTableFlags_Reorderable)) {
							ImGui::TableSetupColumn("Parameter");
							ImGui::TableSetupColumn("Value");
							ImGui::TableHeadersRow();

							for (auto& [id, value] : act.fittedParameters) {
								ImGui::TableNextRow();
								ImGui::TableNextColumn();
								ImGui::Text(data.paramConfig->parameters[id].c_str());
								ImGui::TableNextColumn();

								ImGui::Text("%e", value);
							}

							ImGui::EndTable();
						}

						ImGui::TableNextColumn();

						if (ImGui::BeginTable("Tuned", 2, tableFlags & ~ImGuiTableFlags_Reorderable)) {
							ImGui::TableSetupColumn("Parameter");
							ImGui::TableSetupColumn("Value");
							ImGui::TableHeadersRow();

							for (auto& [id, value] : act.tunedParameters) {
								ImGui::TableNextRow();
								ImGui::TableNextColumn();
								ImGui::Text(data.paramConfig->parameters[id].c_str());
								ImGui::TableNextColumn();
								ImGui::Text("%e", value);
							}

							ImGui::EndTable();
						}

						ImGui::EndTable();
					}

				}

				ImGui::Separator();

				//draw the parameter sliders
				{
					ImGui::PushItemWidth(300);

					if (ImGui::BeginTable("Tuned parameters", 4, ImGuiTableFlags_SizingStretchProp)) {
						ImGui::TableNextRow();

						for (auto& [id, param_value] : act.tunedParameters) {
							// Normalize the value
							int power = std::floor(std::log10(param_value));
							float value = param_value / std::pow(10, power);
							bool checked = act.fixedParameterIDs[id];

							// Unique widget ID
							std::string uniqueID = "##" + std::to_string(id);

							// First column: Checkbox for activation
							ImGui::TableNextColumn();
							ImGui::Checkbox(data.paramConfig->parameters[id].c_str(), &act.tempParametersActive[id]);
							ImGui::PushItemWidth(400);
							// Second column: Value slider
							ImGui::SameLine();
							if (ImGui::SliderFloat((uniqueID + "_slider").c_str(), &value, 1.0f, 9.999f))
								act.toTunne = true;
							
							ImGui::SameLine();
							if (ImGui::SliderInt((uniqueID + "_power").c_str(), &power,  -20 , 20))
								act.toTunne = true;

							ImGui::PopItemWidth();
							// Fourth column: Fixed parameter checkbox
							ImGui::SameLine();
							if (ImGui::Checkbox((uniqueID + "_fixed").c_str(), &checked)) {
								if (checked) 
								{
									act.savedFixedParametersValues[id] = value * std::pow(10, power);
									act.savedFixedParameterIDs[id] = true;
									act.fixedParameterIDs[id] = true;
									act.fixedParametersValues[id] = value * std::pow(10, power);
								}
					/*			else 
								{
									act.savedFixedParameterIDs[id] = false;
									act.fixedParameterIDs[id] = false;
								}*/
								
							}
							
							if (act.toTunne and act.tempParametersActive[id])
							{
								// Update the tuned parameter value
								act.tunedParameters[id] = value * std::pow(10, power);
								act.toTunne = false;
								const auto tmpParameters = act.fittedParameters;
								const auto tmpCurrent = act.fittedI;
								act.fittedParameters = act.tunedParameters;
								// Trigger callback if active
								act.m_tuneCallback();
								act.fittedParameters = tmpParameters;
								act.tunedI = act.fittedI;
								act.fittedI = tmpCurrent;
							}

							ImGui::TableNextRow();
						}

						ImGui::EndTable();
						ImGui::PopItemWidth();
					}
				}

				ImGui::EndChild();


				if (!data.active) {
					ImGui::EndDisabled();
				}
			}
		}
		ImGui::End();
	}
};