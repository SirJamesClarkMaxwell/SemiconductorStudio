#pragma once
#include "components.h"

namespace Components {


	void parameter_input_double(const std::string& pName, double* val) {
		ImGui::InputDouble(pName.c_str(), val, 0.0f, 0.0f, "%e");
	}

	void show_button(bool* pressed) {
		if (pressed == nullptr) return;
		bool disable = *pressed;
		if (disable) ImGui::BeginDisabled();
		*pressed ^= ImGui::Button("Sumbit");
		if (disable) ImGui::EndDisabled();
	}
}