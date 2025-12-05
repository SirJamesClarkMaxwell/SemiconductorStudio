#include "pch.hpp"
#include "AppRunner.hpp"
#include "JFMFitting.hpp"
#include "DataManager.hpp"
#include "tests.hpp"

int main()
{
#if 0
	Tests::test();
#endif
#if 1
	JFMApp::AppRunner runner{};


	std::shared_ptr<JFMService::FittingService::Fitting> fService = std::make_shared<JFMService::FittingService::Fitting>();
	std::shared_ptr<JFMService::DataManager> dService = std::make_shared<JFMService::DataManager>();

	JFMApp::AppServiceBundle bundle{ fService, dService };
	runner.addApp(bundle);

	runner.run();
#endif
	//std::cin.get();
	return 0;
}
