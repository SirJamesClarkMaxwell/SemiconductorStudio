#include <iostream>
#include "tester.hpp"


int main() {

	Platform::Window::initProperties(1920, 1080, "FittingTester");


	
	//TestedFitting to be initialized in the constructor of Tester
	Tester t1{};

	t1.run(UI::Components::draw_fitting_tester);

	return 0;
}