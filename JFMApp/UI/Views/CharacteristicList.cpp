#include "pch.hpp"

#include "Widgets.hpp"

namespace JFMApp::Views {

	void Widgets::CharacteristicList(Data::BrowserData& data) {


		ImGuiChildFlags cf = ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Border;

		ImGui::BeginChild("CListWin", ImVec2{ 0.0f, 0.0f }, cf);

		//control buttons
		{
			ImGui::ColorEdit4("Start Color", &data.startColor.x, ImGuiColorEditFlags_NoInputs);
			ImGui::SameLine();
			ImGui::ColorEdit4("End Color", &data.endColor.x, ImGuiColorEditFlags_NoInputs);

			ImGui::SameLine(0.0f, 20.0f);

			ImGui::BeginGroup();

			if (ImGui::Button("Update colors")) {
				data.m_updateColorsCallback();
			}

			if (ImGui::Button("Invert Selection")) {
				data.m_invertSelectionCallback();
			}

			ImGui::EndGroup();

			ImGui::SameLine(0.0f, 20.0f);

			ImGui::BeginGroup();

			if (ImGui::Button("Select all")) {
				data.m_selectAllCallback();
			}

			if (ImGui::Button("Unselect all")) {
				//data.m_unselectAllCallback();
			}

			ImGui::EndGroup();

			ImGui::SameLine(0.0f, 20.0f);

			ImGui::BeginGroup();

			if (ImGui::Button("Remove selected")) {
				data.m_removeSelectedCallback();
			}

			if (ImGui::Button("Remove unselected")) {
				data.m_removeUnselectedCallback();
			}

			ImGui::EndGroup();
		}


		ImGui::EndChild();

		//characteristics list
		{
			auto* nConf = data.nConfig;

			int param_size = 4 + (nConf ? nConf->parameters.size() : 0);

			ImGuiTableFlags flags = ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchSame;
			ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);

			if (ImGui::BeginTable("Characteristics", param_size, flags, size)) {

				ImGui::TableSetupColumn("Color");
				ImGui::TableSetupColumn("Name");
				ImGui::TableSetupColumn("Error");
				ImGui::TableSetupColumn("T");

				if (param_size > 4)
					for (const auto& [key, val] : nConf->parameters)
						ImGui::TableSetupColumn(val.c_str());

				ImGui::TableHeadersRow();


				int id = 0;
				for (auto& ch : data.m_characteristics) {
					ImGui::TableNextRow();
					

					ImGui::TableNextColumn();
					ImGui::PushID(id++);
					ImGui::ColorEdit4("##color", &ch.color.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
					


					ImGui::TableNextColumn();
					ImGui::Checkbox(ch.name.c_str(), &ch.checked);

					if (ImGui::IsItemHovered())
						ch.weight = 2.0f;
					else
						ch.weight = 1.0f;
					

					ImGui::TableNextColumn();
					std::ostringstream out;
					out << std::scientific << std::setprecision(10) << ch.fitError;
					auto s = out.str();
					ImGui::Text(s.c_str() );

					ImGui::TableNextColumn();
					ImGui::Text(std::to_string(ch.T).c_str());

					if (param_size > 4)
						for (const auto& [key, val] : nConf->parameters) {
							ImGui::TableNextColumn();

							std::string value = ch.fittedParameters.contains(key) ? std::to_string(ch.fittedParameters[key]) : "-";
							ImGui::Text(value.c_str());
						}

					ImGui::PopID();
				}

				ImGui::EndTable();
			}
		}

	}

};
