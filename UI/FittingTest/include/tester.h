#pragma once
#include <iostream>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "sandbox.h"
#include "implot.h"

#include <GLFW/glfw3.h>

class Tester {
	int m_winWindth = 1280;
	int m_winHeight = 720;
	std::string m_winName = "Tester window";
	GLFWwindow* m_window = nullptr;
	const char* m_glsl_version = "#version 130";

	struct Parameters {
		double a = 0.0f;
		double b = 0.0f;
	} input_params;

	static void glfw_error_callback(int error, const char* description)
	{
		std::cout << "GLFW Error %d: %s\n" << error << description << "\n";
	}

public:
	Tester() {}

	Tester(int win_width, int win_height, std::string win_name) 
		: m_winWindth(win_width), m_winHeight(win_height), m_winName(win_name)  {}

	bool init() {
		glfwSetErrorCallback(glfw_error_callback);
		if (!glfwInit()) return false;

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

		this->m_window = glfwCreateWindow(this->m_winWindth, this->m_winHeight, this->m_winName.c_str(), nullptr, nullptr);
		if (this->m_window == nullptr)
			return false;
		glfwMakeContextCurrent(this->m_window);
		glfwSwapInterval(1);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImPlot::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForOpenGL(this->m_window, true);
		ImGui_ImplOpenGL3_Init(this->m_glsl_version);

		return true;
	}

	void run() {
		bool show_demo_window = true;

		while (!glfwWindowShouldClose(this->m_window)) {
			glfwPollEvents();


			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			if (show_demo_window) {
				ImGui::ShowDemoWindow(&show_demo_window);
				ImPlot::ShowDemoWindow(&show_demo_window);
			}
			bool show = true;
			show_test_window(&show);
				

			ImGui::Render();
			int display_w, display_h;
			glfwGetFramebufferSize(this->m_window, &display_w, &display_h);
			glViewport(0, 0, display_w, display_h);
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			glfwSwapBuffers(this->m_window);
		}
	}

	void show_test_window(bool *show) {
		ImGui::Begin("Fitting Tester", show);
		
		parameter_input_double("Parameter a", &(this->input_params.a));
		parameter_input_double("Parameter b", &(this->input_params.b));

		static bool pressed = false;
		show_button(&pressed);

		if (pressed)
			submit_pressed();

		ImGui::End();
	}

	void show_button(bool *pressed) {
		*pressed ^= ImGui::Button("Sumbit");
	}

	void parameter_input_double(const std::string& pName, double* val) {
		ImGui::InputDouble(pName.c_str(), val, 0.0f, 0.0f, "%e");
	}

	void submit_pressed() {
		ImGui::Text("Button pressed");
;	}

	void destroy() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImPlot::DestroyContext();
		ImGui::DestroyContext();

		glfwDestroyWindow(this->m_window);
		glfwTerminate();
	}

	~Tester() {
		destroy();
	}

};