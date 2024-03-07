#include "tester.hpp"

using namespace UI;


void Tester::run(std::function<void(std::shared_ptr<Data::FittingTesterData>)> draw) {
	while (!Platform::Window::shouldClose()) {
		if (!Platform::Window::beginFrame()) return;

		Components::draw_bg_window();

		draw(this->state);
		this->update();

		Platform::Window::endFrame();
	}
}

void Tester::update() {
	this->fitting.update(this->getState());
}


Tester::Tester() {
	//TestedFitting to be initialized here

	//mock code to present the UI

	
	//to be deleted
	this->state = std::make_shared<Data::FittingTesterData>();

	auto& lst = this->state.get()->getFittingOptions()->list;
	lst.push_back("aa");
	lst.push_back("bb");
	lst.push_back("cc");
	lst.push_back("dd");



	this->state.get()->getChisqData()->prev_op = "Contraction";
	this->state.get()->getChisqData()->next_op = "Expansion";

	//plots setup
	int d_points = 400;
	int verts = 6;
	std::vector<double> args{};
	std::vector<double> gvals{};
	std::vector<double> comp{};
	args.resize(d_points);
	gvals.resize(d_points);
	comp.resize(d_points);

	auto& par_names = this->state.get()->getPlots()->parameter_names;
	par_names.push_back("a");
	par_names.push_back("b");
	par_names.push_back("c");

	auto* plots = this->state.get()->getPlots();

	plots->plots.resize(verts);

	for (int p = 0; p < verts; p++) {
		auto& plot = this->state.get()->getPlots()->plots[p];
		plot.comp_values.resize(d_points);
		plot.parameters.push_back((double)p / 5.0 + 1.0);
		plot.parameters.push_back((double)p / 10.0);
		plot.parameters.push_back(3.0);
	}

	for (int v = 0; v < d_points; v++) {
		args[v] = (double)v / (double)d_points;
		gvals[v] = exp(args[v]);

		for (int p = 0; p < verts; p++) {
			auto& plot = this->state.get()->getPlots()->plots[p];

			plot.comp_values[v] = plot.parameters[0] + plot.parameters[2] * powf(args[v], 2);
		}
	}

	
	plots->axis = std::make_pair("I", "V");

	plots->arguments = args;
	plots->given_values = gvals;

	for (int p = 0; p < verts; p++) {
		plots->plots[p].vertex_num = p + 1;
	}



}


std::shared_ptr<Data::FittingTesterData> Tester::getState() {
	return this->state;
}

	