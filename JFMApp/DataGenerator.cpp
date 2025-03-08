#include "Widgets.hpp"

namespace JFMApp::Views {

	void Widgets::DataGenerator(Data::BrowserData& data)
	{
		ImVec2 size = { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y };
		ImGui::BeginChild("Data Generator", size, true);
		ImGui::PushItemWidth(size.x * 0.20f);
		if (ImGui::BeginCombo("Model", data.m_genModelID ? data.nConfig->models[data.m_genModelID].c_str() : "Select Model"))
		{
			for (const auto& [id, model] : data.nConfig->models)
			{
				if (ImGui::Selectable(model.c_str(), id == data.m_genModelID))
					data.m_genModelID = id;
			}
			ImGui::EndCombo();
		}

		ImGui::SameLine(0.0f, 20.0f);

		if (ImGui::RadioButton("Number of steps", data.m_byStepN))
			data.m_byStepN = true;
		ImGui::SameLine();
		if (ImGui::RadioButton("Step size", !data.m_byStepN))
			data.m_byStepN = false;

		ImGui::SameLine(0.0f, 20.0f);
		if (ImGui::InputFloat("Temperature", &data.m_genT, 0.1, 1.0, "%.1f") && ImGui::IsItemDeactivatedAfterEdit())
			data.m_genT = std::clamp(data.m_genT, 0.0f, 500.0f);

		ImGui::SameLine(0.0f, 20.0f);
		float n = data.m_noise;
		if (ImGui::InputFloat("Noise", &n, 0.1, 1.0, "%.1f") && ImGui::IsItemDeactivatedAfterEdit())
			data.m_noise = std::clamp(n, 0.0f, 100.0f);

		if (ImGui::DragFloat2("Temperature Range", data.m_tempRange.data(), 1.0f, 0.001f, 10.0f) && ImGui::IsItemDeactivatedAfterEdit()) {
			data.m_tempRange[0] = std::clamp(data.m_tempRange[0], 1.0f, data.m_tempRange[1]);
			data.m_tempRange[1] = std::clamp(data.m_tempRange[1], data.m_tempRange[0], 500.0f);
		}
		ImGui::SameLine(0.0f, 20.0f);
		if (ImGui::InputScalar("T number", ImGuiDataType_U32, &data.m_tempN, nullptr, nullptr, "%d", ImGuiInputTextFlags_CharsDecimal) && ImGui::IsItemDeactivatedAfterEdit())
			data.m_tempN = std::clamp(data.m_tempN, (size_t)1, (size_t)1000);



		if (ImGui::DragFloat2("Noise Range", data.m_noiseRange.data(), 1.0f, 0.001f, 10.0f) && ImGui::IsItemDeactivatedAfterEdit()) {
			data.m_noiseRange[0] = std::clamp(data.m_noiseRange[0], 0.0f, data.m_noiseRange[1]);
			data.m_noiseRange[1] = std::clamp(data.m_noiseRange[1], data.m_noiseRange[0], 100.0f);
		}
		ImGui::SameLine(0.0f, 20.0f);
		if (ImGui::InputScalar("Noise number", ImGuiDataType_U32, &data.m_noiseN, nullptr, nullptr, "%d", ImGuiInputTextFlags_CharsDecimal) && ImGui::IsItemDeactivatedAfterEdit())
			data.m_noiseN = std::clamp(data.m_noiseN, (size_t)1, (size_t)1000);

		if (ImGui::DragFloat2("Voltage Range", data.m_voltageGenRange.data(), 1.0f, 0.001f, 10.0f) && ImGui::IsItemDeactivatedAfterEdit()) {
			data.m_voltageGenRange[0] = std::clamp(data.m_voltageGenRange[0], 0.001f, data.m_voltageGenRange[1]);
			data.m_voltageGenRange[1] = std::clamp(data.m_voltageGenRange[1], data.m_voltageGenRange[0], 10.0f);
		}

		ImGui::SameLine();
		if (data.m_byStepN) {
			if (ImGui::InputScalar("N", ImGuiDataType_U32, &data.m_nSteps, nullptr, nullptr, "%d", ImGuiInputTextFlags_CharsDecimal) && ImGui::IsItemDeactivatedAfterEdit())
				data.m_nSteps = std::clamp(data.m_nSteps, (size_t)5, (size_t)1000);
		}
		else {
			if (ImGui::InputFloat("Step", &data.m_voltageGenStep, 0.001, 0.1, "%e") && ImGui::IsItemDeactivatedAfterEdit())
				data.m_voltageGenStep = std::clamp(data.m_voltageGenStep, 0.001f, data.m_voltageGenRange[1] - data.m_voltageGenRange[0]);
		}

		ImGui::PopItemWidth();
		ImGui::PushItemWidth(size.x * 0.20f);

		if (data.m_genModelID)
			for (auto& param : data.nConfig->modelParameters[data.m_genModelID]) {
				if (ImGui::BeginCombo(data.nConfig->parameters[param].c_str(), data.m_genTypes[data.m_paramGenData[param].type].c_str()))
				{
					for (unsigned int t = 0; t < data.m_genTypes.size(); t++)
					{
						if (ImGui::Selectable(data.m_genTypes[t].c_str(), t == data.m_paramGenData[param].type))
							data.m_paramGenData[param].type = (Data::BrowserData::GenType)t;
					}

					ImGui::EndCombo();
				}
			}

		ImGui::PopItemWidth();

		ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedFit;
		ImVec2 tsize = { ImGui::GetContentRegionAvail().x, 0.0f };

		if (data.m_genModelID && ImGui::BeginTable("Generate config", 5, flags, tsize)) {

			ImGui::TableSetupColumn("Enable Range", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Start", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("End", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn(data.m_byStepN ? "N" : "Step size", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Single Value", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableHeadersRow();


			for (auto& param : data.nConfig->modelParameters[data.m_genModelID]) {
				ImGui::TableNextRow();
				if (param == *data.nConfig->modelParameters[data.m_genModelID].begin()) {
					ImGui::TableSetColumnIndex(0);
					ImGui::TableSetColumnIndex(1);
					ImGui::TableSetColumnIndex(2);
					ImGui::TableSetColumnIndex(3);
					ImGui::TableSetColumnIndex(4);
				}

				ImGui::PushID(param);

				ImGui::TableSetColumnIndex(0);
				ImGui::Checkbox(data.nConfig->parameters[param].c_str(), &data.m_paramGenData[param].singleShot);
				ImGui::TableSetColumnIndex(1);
				if (!data.m_paramGenData[param].singleShot)
					ImGui::BeginDisabled();

				if(ImGui::InputDouble("##start", &data.m_paramGenData[param].start, 0.001, 0.1, "%e") && ImGui::IsItemDeactivatedAfterEdit())
					data.m_paramGenData[param].start = std::clamp(data.m_paramGenData[param].start, data.nConfig->paramBounds[param].first, data.m_paramGenData[param].end);

				ImGui::TableSetColumnIndex(2);
				if (ImGui::InputDouble("##end", &data.m_paramGenData[param].end, 0.001, 0.1, "%e") && ImGui::IsItemDeactivatedAfterEdit())
					data.m_paramGenData[param].end = std::clamp(data.m_paramGenData[param].end, data.m_paramGenData[param].start, data.nConfig->paramBounds[param].second);

				ImGui::TableSetColumnIndex(3);
				if (data.m_byStepN) {
					if (ImGui::InputScalar("##n", ImGuiDataType_U32, &data.m_paramGenData[param].nSteps, nullptr, nullptr, "%d", ImGuiInputTextFlags_CharsDecimal) && ImGui::IsItemDeactivatedAfterEdit())
						data.m_paramGenData[param].nSteps = std::clamp(data.m_paramGenData[param].nSteps, (size_t)1, (size_t)10000);
				}
				else {
					if (ImGui::InputDouble("##step", &data.m_paramGenData[param].step, 0.001, 0.1, "%e") && ImGui::IsItemDeactivatedAfterEdit())
						data.m_paramGenData[param].step = std::clamp(data.m_paramGenData[param].step, 0.001, data.m_paramGenData[param].end - data.m_paramGenData[param].start);
				}

				if (!data.m_paramGenData[param].singleShot)
					ImGui::EndDisabled();

				ImGui::TableSetColumnIndex(4);
				if (ImGui::InputDouble("##single", &data.m_paramGenData[param].singleValue, 0.001, 0.1, "%e") && ImGui::IsItemDeactivatedAfterEdit())
					data.m_paramGenData[param].singleValue = std::clamp(data.m_paramGenData[param].singleValue, data.nConfig->paramBounds[param].first, data.nConfig->paramBounds[param].second);

				ImGui::PopID();
			}

			ImGui::EndTable();
		}

		if (ImGui::Button("Generate single")) {
			data.m_singleShot();
		}

		ImGui::SameLine(0.0f, 20.0f);

		if (ImGui::Button("Generate range")) {
			data.m_generateCallback();
		}

		ImGui::EndChild();
	}
};