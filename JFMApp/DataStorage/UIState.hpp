#pragma once

#include "pch.hpp"

namespace JFMApp::Data
{
	struct UIState
	{
		bool m_showPlottingArea{ true };
		bool m_showBrowserArea{ true };
		bool m_showCharacteristicInspector{ true };
		bool m_showMonteCarloInspector{ false };
		bool m_showGenerator{ false };
		bool m_showArrheniusViewer{ true };
		
	};
}


