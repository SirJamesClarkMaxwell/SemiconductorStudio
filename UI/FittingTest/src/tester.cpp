#include "tester.h"

void Tester::glfw_error_callback(int error, const char* description)
{
	std::cout << "GLFW Error %d: %s\n" << error << description << "\n";
}

void Tester::show_test_window(bool* show) {
	ImGui::Begin("Fitting Tester", show);

	Components::parameter_input_double("Parameter a", &(this->input_params.a));
	Components::parameter_input_double("Parameter b", &(this->input_params.b));

	static bool pressed = false;
	Components::show_button(&pressed);

	if (pressed)
		submit_pressed(&pressed);

	ImGui::End();
}

void Tester::submit_pressed(bool *disabled) {
	static double start_time = glfwGetTime();
	static bool first_call = true;
	if (first_call) {
		first_call = false;
		start_time = glfwGetTime();
	}

	double time = glfwGetTime();
	ImGui::Text(std::to_string(time - start_time).c_str());
	if (time - start_time > 10.0) {
		*disabled = false;
		first_call = true;
	} 

}

void Tester::destroy() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImPlot::DestroyContext();
	ImGui::DestroyContext();

	glfwDestroyWindow(this->m_window);
	glfwTerminate();
}

void Tester::beginFrame() {
	glfwPollEvents();

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void Tester::endFrame() {
	ImGui::Render();
	int display_w, display_h;
	glfwGetFramebufferSize(this->m_window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(this->m_window);
}

void Tester::draw() {
	static bool show_demo_window = true;
	if (show_demo_window) {
		ImGui::ShowDemoWindow(&show_demo_window);
		ImPlot::ShowDemoWindow(&show_demo_window);
	}
	static bool show = true;
	show_test_window(&show);
}

bool Tester::shouldStop() {
	return this->m_stop || glfwWindowShouldClose(this->m_window);
}


bool Tester::init() {
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

void Tester::run() {

	while (!this->shouldStop()) {
		this->beginFrame();

		this->draw();

		this->endFrame();
	}
}

	