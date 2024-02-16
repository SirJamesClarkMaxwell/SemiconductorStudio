#pragma once
#include <iostream>
#include <string>
//#include <chrono>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "sandbox.h"
#include "implot.h"
#include "components.h"


#include <GLFW/glfw3.h>

class Tester {
	int m_winWindth = 1280;
	int m_winHeight = 720;
	std::string m_winName = "Tester window";
	GLFWwindow* m_window = nullptr;
	const char* m_glsl_version = "#version 130";

	bool m_stop = false;

	struct Parameters {
		double a = 0.0f;
		double b = 0.0f;
	} input_params;

	static void glfw_error_callback(int error, const char* description);
	void show_test_window(bool* show);
	void submit_pressed(bool *disabled);
	void destroy();
	void beginFrame();
	void endFrame();
	void draw();
	bool shouldStop();

public:
	Tester() {}
	Tester(int win_width, int win_height, std::string win_name)
		: m_winWindth(win_width), m_winHeight(win_height), m_winName(win_name) {}

	bool init();
	void run();

	~Tester() {
		this->destroy();
	}
};