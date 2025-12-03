#include "pch.hpp"
#include "Characteristic.hpp"

namespace JFMApp::Data
{
	Characteristic::Characteristic(const CharacteristicData &loaded)
	{

		V = loaded.Data[CharacteristicData::Voltage];
		I = loaded.Data[CharacteristicData::Current];
		T = loaded.Temperature;
		name = loaded.Name;
		modelID = 3;
		savedModelID = modelID;
		initState();
	}

	void Characteristic::initState()
	{
		mcMutex = std::make_shared<std::mutex>();
	}

	EstimateInput Characteristic::getEstimateInput()
	{
		EstimateInput input{};

		std::span<double> eV{V.begin() + dataRange.first, V.begin() + dataRange.second};
		std::span<double> eI{I.begin() + dataRange.first, I.begin() + dataRange.second};

		// FIX this in the future - abstract parameters in the loading service
		int index = JFMService::Fitters::AdditionalParametersID::Temperature;
		input.additionalParameters[index] = T;
		input.modelID = savedModelID;
		input.characteristic = {eV, eI};

		return input;
	}

	FittingInput Characteristic::getFittingInput()
	{
		FittingInput input{};

		input.initialData = getEstimateInput();
		input.name = name;
		input.fixConfig = savedFixedParametersValues;
		for (auto &[id, val] : savedFixedParameterIDs)
		{
			if (!val)
				input.fixConfig.erase(id);
		}

		if (savedUseBounds)
			input.bounds = savedBounds;

		if (savedUseInitial)
			input.initialValues = savedInitialGuess;

		input.useBounds = savedUseBounds;

		return input;
	}

	void Characteristic::submitFitting(const ParameterMap &parameters, double error)
	{
		fittedParameters = parameters;
		//tunedParameters = parameters;
		fixedParametersValues = parameters;
		savedFixedParametersValues = parameters;
		for (const auto &[k, v] : parameters)
		{
			fixedParameterIDs[k] = false;
			savedFixedParameterIDs[k] = false;
		}
		initialGuess = parameters;
		savedInitialGuess = parameters;
		//tunedI = fittedI;
		isFitted = true;
		fitError = error;
	}

	CalculatingData Characteristic::getCalculatingData()
	{
		CalculatingData data{};
		data.characteristic = getEstimateInput().characteristic;
		fittedI.resize(data.characteristic.currentData.size());
		data.characteristic.currentData = std::span<double>{fittedI};
		data.parameters = fittedParameters;
		data.additionalParameters = {{6, T}};
		data.modelID = modelID;
		return data;
	}

	CalculatingData Characteristic::getTuningData()
	{
		CalculatingData data{};
		data.characteristic = getEstimateInput().characteristic;
		tunedI.resize(data.characteristic.currentData.size());
		data.characteristic.currentData = std::span<double>{tunedI};
		data.parameters = tunedParameters;
		data.additionalParameters = {{6, T}};
		data.modelID = modelID;
		return data;
	}

	MCInput Characteristic::getMCConfig()
	{
		MCInput input;
		input.firstFitError = fitError;
		input.startingData = getFittingInput();
		std::string mc_name = name;
		input.startingData.name = name;
		for (auto &[k, v] : input.startingData.fixConfig)
		{
			mc_name += "_";
			mc_name += nConfig.parameters[k];
		}
		mc_name += ".yaml";
		input.relPath = mc_name;
		input.trueParameters = fittedParameters;
		input.iterations = savedMCConfig.n;
		input.noise = savedMCConfig.sigma;
		return input;
	}

	void Characteristic::submitMC(MCOutput &&out, int /* ignore */)
	{
		MCSimulation sim{};

		sim.data.resize(out.mcResult.size());
		for (const auto &[src, dest] : std::views::zip(out.mcResult, sim.data))
			dest = std::move(src);
        out.mcResult.clear();
		std::sort(sim.data.begin(), sim.data.end(), [](const MCData &lhs, const MCData &rhs)
				  { return lhs.error > rhs.error; });

        doSubmitMC(sim, out);
	}

	void Characteristic::submitMC(const MCOutput &out)
	{
		MCSimulation sim{};
		sim.data.resize(out.mcResult.size());
		for (const auto &[src, dest] : std::views::zip(out.mcResult, sim.data))
		{
			dest = std::move(src);
		}
		std::sort(sim.data.begin(), sim.data.end(), [](const MCData &lhs, const MCData &rhs)
				  { return lhs.error > rhs.error; });

        doSubmitMC(sim, out);
	}

    void Characteristic::doSubmitMC(MCSimulation &sim, const MCOutput &out)
    {
		sim.sigma = out.inputData.noise;
		sim.iterations = out.inputData.iterations;
		sim.fixConfig = out.inputData.startingData.fixConfig;
		sim.sim_name = out.inputData.relPath.filename().string();
		sim.relPath = path.parent_path();
		sim.trueParameters = out.inputData.trueParameters;
		sim.modelID = modelID;
		std::scoped_lock lk{*mcMutex};

		//auto d = std::find_if(mcData.begin(), mcData.end(), [&](MCSimulation &d)
		//					  { return d.fixConfig == sim.fixConfig; });
		//if (d == mcData.end())
		mcData.push_back(sim);
		//else
		//	*d = sim;
    }

	std::string Characteristic::buildMCPlotName(ParameterID xId,ParameterID yId, int iterations,double noise)
	{
		std::string toReturn = name;
		toReturn +="_"+ nConfig.parameters[xId] +"_"+ nConfig.parameters[yId];
		toReturn += "_" + std::to_string(iterations) +"_"+std::to_string(noise);
		return toReturn;
	}
}
