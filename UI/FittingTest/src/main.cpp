#include <iostream>
#include "tester.hpp"


//TODO
//Manage global config og the UI		--DONE
//Design simulated annealing options
//Look for explorer solutions
//Make plot height equvivalent to width --DONE
//Revise the data structs
//Selection of text(maybe with ctrl+click or shift+click
//Imrove double formatting options(low priority)
//manage docking properly
//





int main() {

	Platform::Window::initProperties(1920, 1080, "FittingTester");


	
	//TestedFitting to be initialized in the constructor of Tester
	Tester t1{};

	t1.run(UI::Components::draw_fitting_tester);

	return 0;
}