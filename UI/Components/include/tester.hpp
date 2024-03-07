#pragma once
#include "includes.hpp"
#include "TestedFitting.hpp"



using namespace UI;

class Tester {
	std::shared_ptr<Data::FittingTesterData> state{ nullptr };
	TestedFitting fitting{};
	void update();

public:

	Tester();

	Tester(const Tester& t) = delete;
	Tester& operator=(const Tester& t) = delete;

	Tester(Tester&& t) noexcept = default;
	Tester& operator=(Tester&& t) noexcept = default;
	~Tester() = default;

	std::shared_ptr<Data::FittingTesterData> getState();

	void run(std::function<void(std::shared_ptr<Data::FittingTesterData>)> draw);
	

};