#pragma once
#include <iostream>
#include "imgui.h"

namespace Components {
	void parameter_input_double(const std::string& pName, double* val);
	void show_button(bool* pressed);
}