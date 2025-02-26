#include "JFMFitting.hpp"
#include "CalculateData.hpp"
#include "IDataManager.hpp"
#include <cstdio>
namespace JFMService::FittingService
{
#define ID_TO_NAME(ID) return #ID

	Fitting::Fitting()
		: m_numericsConfig(instantiateNumericsConfig())
	{
	}
	NumericsConfig Fitting::GetConfiguration()
	{
		return m_numericsConfig;
	}

	void Fitting::CalculateData(CalculatingData &input)
	{
		m_dataCalculator.CalculateData(input);
	}

	double Fitting::CalculateError(const std::span<double> &original, const std::span<double> &checked)
	{
		return m_dataCalculator.CalculateError(original, checked);
	}

	std::pair<size_t, size_t> Fitting::RangeData(const PlotData &characteristic)
	{
		return m_preFitter.RangeData(characteristic);
	}

	std::unordered_map<ParameterID, double> Fitting::Estimate(const EstimateInput &input)
	{
		return m_preFitter.Estimate(input);
	}

	NumericsConfig Fitting::instantiateNumericsConfig()
	{
		NumericsConfig config;
		auto intRange = [&](double min, double max, double step)
		{
			int size = static_cast<int>((max - min) / step) + 1;
			std::vector<unsigned int> destination(size);

			int count = -1;
			std::ranges::generate(destination.begin(), destination.end(), [&]()
								  { count++; return min + count * step; });
			return destination;
		};
		auto createModelMap = [&]()
		{
			auto ModelsToParameters = [](int id)
			{
				switch (id)
				{
				case Model4P:
					return "FourParameterModel";
				case Model6P:
					return "SixParameterModel";

				default:
					break;
				}
			};
			ModelParameters map;
			map[Model4P] = intRange(0, Model4P, 1);
			map[Model6P] = intRange(0, Model6P, 1);
			return map;
		};
		auto createParameterMap = [&]()
		{
			JFMService::FittingService::Parameters map;
			std::string name;
			for (auto index : std::ranges::iota_view(0, Fitters::ParameterID::p_size))
				map[index] = DataManagementService::parameterIdToString((Fitters::ParameterID)index);
			return map;
		};
		auto createModelsParameterMap = [&]()
		{
			ModelParameters map;
			map[Model4P] = intRange(0, Model4P, 1);
			map[Model6P] = intRange(0, Model6P, 1);
			return map;
		};
		auto createGeneralBounds = [&]()
		{
			// todo adjust proper values for alpha and Rsh2
			//! order A,I0,Rs,Rsh,alpha,Rsh2
			std::vector<double> minBound{0.5, 1e-20, 1e-7, 10, 0, 0};
			std::vector<double> maxBound{20, 1e-3, 1e2, 1e9, 10, 1e-10};
			ParamBounds bounds;
			for (const auto &[id, min, max] : std::views::zip(std::ranges::iota_view(0, Fitters::ParameterID::p_size), minBound, maxBound))
				bounds[id] = std::make_pair(min, max);
			return bounds;
		};
		auto createModel = [&]()
		{
			FittingService::Models models;
			models[Model4P] = "Four Parameter";
			models[Model6P] = "Six Parameter";
			return models;
		};
		config.models = createModel();
		config.modelParameters = createModelMap();
		config.parameters = createParameterMap();
		config.modelParameters = createModelsParameterMap();
		config.paramBounds = createGeneralBounds();
		return config;
	};

	void Fitting::Fit(const FittingInput &input, std::function<void(ParameterMap &&)> callback) { m_fitter.Fit(input, callback); }

	void Fitting::Simulate(const MCInput &input, std::function<void(MCOutput &&)> callback)
	{
		m_monteCarlo.Simulate(input, callback);
	}
	double Fitting::GetUncertainty(const MCOutput &output, int level, ParameterID id)
	{
		return m_monteCarlo.GetUncertainty(output, level, id);
	}
	void Fitting::SaveMCPlot(const MCSave &toSave)
	{
		std::string fname = toSave.pathToSave.filename().string() + ".txt";
		const std::filesystem::path path = toSave.pathToSave.parent_path() / fname;

		std::vector<double> xSave, ySave, errors;
		ParameterID xID{toSave.x_label}, yID{toSave.y_label};
		for (const auto &item : toSave.results)
		{
			xSave.push_back(item.foundParameters.at(xID));
			ySave.push_back(item.foundParameters.at(yID));
			errors.push_back(item.error);
		}
		std::string dataToSave;
		dataToSave += std::to_string(toSave.degreesOfFreedom) + "\n";
		dataToSave += toSave.title + "\n";
		using namespace DataManagementService;

		dataToSave += std::string(parameterIdToString((Fitters::ParameterID)toSave.x_label)) + "\n";
		dataToSave += std::string(parameterIdToString((Fitters::ParameterID)toSave.y_label)) + "\n";
		auto formatData = [](double x, double y, double e)
		{
			auto ToScientific = [](double value)
			{
				std::ostringstream out;
				out << std::scientific << std::setprecision(6) << value;
				return out.str();
			};
			std::string toReturn;
			toReturn += ToScientific(x) + ", ";
			toReturn += ToScientific(y) + ", ";
			toReturn += ToScientific(e) + "\n";
			return toReturn;
		};

		for (const auto &[x, y, e] : std::views::zip(xSave, ySave, errors))
			dataToSave += formatData(x, y, e);
		if (!std::filesystem::exists(path))
			std::filesystem::create_directories(path);
		std::ofstream file(path);
		file << dataToSave;
		file.close();
		std::string command = "python ./generate_image.py " + path.string();
		std::system(command.c_str());
#if 1		
		std::cout << path.string();
		try {
			// Attempt to remove the file
			if (std::filesystem::remove(path)) {
				std::cout << "File deleted successfully.\n";
			}
			else {
				std::cout << "File not found.\n";
			}
		}
		catch (const std::filesystem::filesystem_error& e) {
			// Catch the exception and print detailed error info
			std::cerr << "Filesystem error: " << e.what() << '\n';
			std::cerr << "Path: " << e.path1() << '\n';

			// Optionally, if the exception involves two paths (e.g., copy operations)
			if (!e.path2().empty()) {
				std::cerr << "Other path: " << e.path2() << '\n';
			}
		}
#endif
	}
	void Fitting::SaveUncertanties(const std::vector<UncertaintySave> &toSave, const std::filesystem::path &path)
	{
		std::stringstream sttringStream;
		std::string xlabel = Fitters::parameterIdToString((Fitters::ParameterID)(*toSave.front().paramPair.begin()).first);
		std::string ylabel = Fitters::parameterIdToString((Fitters::ParameterID)(*(--toSave.front().paramPair.end())).first);

		sttringStream << "Name\tTemperature\t1/T\t";
		for (const auto& [key, val] : toSave.front().paramPair)
			sttringStream <<"ln("<< Fitters::parameterIdToString((Fitters::ParameterID)key)<<")" << "\t66%\t95%\t99%\t";
		sttringStream << std::endl;

		auto SerializeUncertaintyType = [](const UncertaintySave &save)
		{
			auto ToScientific = [](double value,bool ln=false)
			{
				std::ostringstream out;
				if (ln)
					out << std::scientific << std::setprecision(6) << std::log(value);
				else
					out << std::scientific << std::setprecision(6) << value;
				return out.str();
			};
			std::string string;
			string += save.name + "\t";
			string += std::to_string(save.T) + "\t"+std::to_string(1/save.T) + "\t";
			for (const auto &[key, val] : save.paramPair)
				string += ToScientific(val) + "\t";
			for (int i=0;i<4;i++)
			{
				for (const auto& item : save.uncertainty)
					string += ToScientific(item.at(i)) + "\t";
			}
			return string + "\n";
		};
		for (const auto &item : toSave)
			sttringStream << SerializeUncertaintyType(item);

		std::ofstream file(path);
		file << sttringStream.str();
		file.close();
	}
}