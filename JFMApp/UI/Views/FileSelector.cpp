#include "pch.hpp"

#include "Widgets.hpp"

namespace JFMApp::Views {
	void Widgets::FileSelector(Data::BrowserData& data)
	{
		ImGuiChildFlags child_flags = ImGuiChildFlags_Border;
		ImGuiWindowFlags win_flags = ImGuiWindowFlags_MenuBar;

		ImGui::BeginChild("Browser", ImVec2{ 0.0, ImGui::GetContentRegionAvail().y }, child_flags, win_flags);
		if (ImGui::BeginMenuBar()) {
			if (ImGui::Button("<-") /*&& data.currentPath != data.rootPath*/) {
				data.m_selection.clear();
				data.currentPath = data.currentPath.parent_path();
				data.m_selection.resize(std::distance(std::filesystem::directory_iterator(data.currentPath), std::filesystem::directory_iterator{}));
			}

			if (ImGui::Button("Load")) {
				data.m_loadCallback();
			}

			if (ImGui::Button("Load all")) {
				data.m_loadAllCallback();
			}
            if (ImGui::BeginCombo("Loading Characteristic Type", data.m_characteristicTypeMap[data.m_characteristicType].c_str(), ImGuiComboFlags_WidthFitPreview))
            {
				for (const auto& [id, name] : data.m_characteristicTypeMap) 
				{
					if (ImGui::Selectable(name.c_str())) 
						data.m_characteristicType = id;
					
				}
				ImGui::EndCombo();
            }
			ImGui::EndMenuBar();
		}


		float width = ImGui::GetContentRegionAvail().x;
		float size = 100.0f;
		int numItems = std::max((int)(width / size / 2.0f), 1);

		ImVec2 selSize = ImVec2(size * 2.0f, size / 2.0f);

		ImVec2 tSize = ImVec2(0.0f, ImGui::GetContentRegionAvail().y);


		if (ImGui::BeginTable("Browser", numItems, ImGuiTableFlags_BordersOuter, tSize)) {

			ImGui::TableNextRow();

			size_t sel_index{ 0 };

			for (auto& entry : std::filesystem::directory_iterator(data.currentPath)) {
				ImGui::TableNextColumn();
				if (entry.is_directory())
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));

				std::string name = entry.path().filename().string();
				bool t = data.m_selection[sel_index++];
				if (ImGui::Selectable(name.c_str(), t, 0, selSize)) {
					if (!ImGui::GetIO().KeyCtrl || entry.is_directory()) {
						for (const auto& sel : data.m_selection)
							sel = false;

					}
					data.m_selection[sel_index - 1] = !data.m_selection[sel_index - 1];
				}

				if (entry.is_directory())
					ImGui::PopStyleColor();


				//if the entry is clicked
				//  if the entry is a directory -> selected, all other cleared
				//	if ctrl pressed
				//		if the entry is a file -> selected
				//	if ctrl not pressed
				//		if the entry is a file -> selected, all other cleared
				//
				//if the entry is double clicked
				//	if the entry is a directory -> current path = entry.path(), all selection cleared
				//	if the entry is a file -> callback




				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {

					if (entry.is_directory()) {
						data.currentPath = entry.path();
						data.m_selection.clear();
						data.m_selection.resize(std::distance(std::filesystem::directory_iterator(data.currentPath), std::filesystem::directory_iterator{}));
						break;
					}
					else {
						for (const auto& sel : data.m_selection)
							sel = false;

						data.m_selection[sel_index - 1] = true;

						data.m_loadCallback();
					}

				}
				
			}

			ImGui::EndTable();
		}


		ImGui::EndChild();

	}
}