#include <iostream>
#include "tester.h"


int main() {

	Tester win{};

	if (!win.init()) return -1;
	win.run();


	return 0;
}