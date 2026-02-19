#pragma once

#include "pch.hpp"
#include "ViewData.hpp"
#include "JFMIFitting.hpp"
#include "IDataManager.hpp"

#include "Widgets.hpp"
#include "imgui_internal.h"

namespace JFMApp {

	struct AppServiceBundle {
		std::shared_ptr<JFMService::FittingService::IFitting> numerics{};
		std::shared_ptr<JFMService::DataManagementService::IDataManager> dataLoader{};
	};

	class App
	{
	public:
		App() = delete;
		App(const AppServiceBundle& services);
		~App() = default;

		App(const App&) = delete;
		App& operator=(const App&) = delete;

		App(App&&) = delete;
		App& operator=(App&&) = delete;

		void init();

		void update();
		void draw();

	private:
		void setUpCallbacks();

	private:
		std::mutex m_charMutex{};
		Data::ViewData m_state{};
		std::shared_ptr<JFMService::FittingService::IFitting> m_numerics{};
		std::shared_ptr<JFMService::DataManagementService::IDataManager> m_dataLoader{};
	};
}



