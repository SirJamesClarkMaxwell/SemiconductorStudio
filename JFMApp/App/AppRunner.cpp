#include "pch.hpp"
#include "AppRunner.hpp"
#include "Widgets.hpp"


namespace JFMApp {
	AppRunner::AppRunner()
	{
		Window::initProperties(1920, 1080, "Junction Fit Master");
	}

	void AppRunner::addApp(const AppServiceBundle& services)
	{
		m_apps.push_back(std::make_unique<App>(services));
	}

	void AppRunner::run()
	{
		while (!Window::shouldClose()) {
			if (!Window::beginFrame())
				return;

			for (auto& app : m_apps)
			{
				app->update();

				JFMApp::Views::Widgets::global_ds();
				
				app->draw();
			}

			Window::endFrame();
		}
	
	}
};