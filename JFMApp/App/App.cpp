#include <map>
#include "pch.hpp"
#include "App.hpp"

std::vector<std::pair<std::vector<double>, std::vector<double>>> globalNoisyI{};
std::vector<std::pair<std::vector<double>, std::vector<double>>> globalErrors{};

namespace JFMApp
{

	using namespace JFMService::DataManagementService;
	using namespace JFMService::FittingService;

	App::App(const AppServiceBundle &services)
	{
		m_numerics = services.numerics;
		m_dataLoader = services.dataLoader;

		init();
	}

	void App::init()
	{
		// if (!m_numerics || !m_dataLoader) return;

		// get the numerics config
		m_state.nConfig = m_numerics->GetConfiguration();

		ImGui::CreateContext();
		ImGuiIO &io = ImGui::GetIO();

		m_state.plotData.mcTabs.push_back(1);

		// Disable the .ini file by setting IniFilename to nullptr
		io.IniFilename = nullptr;

		// setup all of the pointers
		// plot data
		m_state.plotData.characteristics = &m_state.browserData.m_characteristics;
		m_state.plotData.paramConfig = &m_state.nConfig;

		// browser data
		m_state.browserData.nConfig = &m_state.nConfig;

		// provide the callbacks
		setUpCallbacks();

		// init the root path
		m_state.browserData.rootPath = std::filesystem::current_path();
		m_state.browserData.currentPath = m_state.browserData.rootPath;

		// init the selection vector for the file browser
		m_state.browserData.m_selection.resize(std::distance(std::filesystem::directory_iterator(m_state.browserData.rootPath), std::filesystem::directory_iterator{}));

		m_state.plotData.globalModelID = 3;
		m_state.plotData.savedGlobalModelID = 3;
	}

	void App::draw()
	{
		// if (!m_numerics || !m_dataLoader) return;
		std::scoped_lock lk{m_charMutex};
		// get the mainviewport dockspace

		ImGuiID mainDockID = Views::Widgets::mDS;

		// dock prograpatically the main window
		ImGui::SetNextWindowDockID(mainDockID, ImGuiCond_Once);
		ImGuiWindowFlags JFMWFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse;

		ImGuiID jfmID = ImGui::GetID("JFM");
		ImGui::Begin("JFM", nullptr, JFMWFlags);

		// ViewMenu is the menu of the main window

		Views::Widgets::ViewMenu(m_state.uiState);

		ImGui::DockSpace(jfmID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

		ImGui::End();
		static bool docked = false;
		if (!docked)
		{
			docked = true;
			ImGui::DockBuilderRemoveNode(jfmID);
			ImGui::DockBuilderAddNode(jfmID, ImGuiDockNodeFlags_DockSpace);
			ImGui::DockBuilderSetNodeSize(jfmID, ImGui::GetMainViewport()->Size);

			ImGuiID dock_main_id = jfmID;
			ImGuiID right, left;
			ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.6f, &left, &right);

			ImGuiID rtop, rbottom;
			ImGui::DockBuilderSplitNode(right, ImGuiDir_Up, 0.5f, &rtop, &rbottom);
			ImGuiID ltop, lbottom;
			ImGui::DockBuilderSplitNode(left, ImGuiDir_Up, 0.8f, &ltop, &lbottom);

			ImGuiID cbTop, cbBottom;
			ImGui::DockBuilderSplitNode(rtop, ImGuiDir_Up, 0.5f, &cbTop, &cbBottom);

			ImGui::DockBuilderDockWindow("Plot Area", ltop);
			ImGui::DockBuilderDockWindow("Characteristic Settings", cbBottom);
			ImGui::DockBuilderDockWindow("Characteristic List", cbTop);
			ImGui::DockBuilderDockWindow("File Explorer", cbBottom);
			ImGui::DockBuilderDockWindow("Characteristic Inspector", rbottom);
			ImGui::DockBuilderDockWindow("Arrhenius Plots", rbottom);
			ImGui::DockBuilderDockWindow("Generator", ltop);

			ImGui::DockBuilderFinish(jfmID);

			std::string id = "MC Tab Dock" + std::to_string(1);
			m_state.plotData.tabsIDs.push_back(ImGui::GetID(id.c_str()));
		}

		// Next plotting area
		if (m_state.uiState.m_showPlottingArea)
		{

			Views::Widgets::PlottingArea(m_state.plotData);
		}
		// next content browser
		if (m_state.uiState.m_showBrowserArea)
		{
			Views::Widgets::BrowserArea(m_state.browserData);
		}

		//	Characteristics inspector - by default
		if (m_state.uiState.m_showCharacteristicInspector)
		{
			if(ImGui::Begin("Characteristic Inspector"));
			{
				Views::Widgets::CharacteristicInspector(m_state.plotData);
			}
				ImGui::End();
		}
		/*
		ImGui::SetNextWindowDockID(mainDockID, ImGuiCond_Once);
		if (ImGui::Begin("RT MC"))
		{
			static int curr_c = 0;
			ImVec2 s = {ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y};
			if (ImPlot::BeginPlot("RT MC", s))
			{
				ImPlot::SetupAxes("V", "I", Data::PlotData::plotSettings.xFlags, Data::PlotData::plotSettings.yFlags);
				
				ImPlot::SetupAxisScale(ImAxis_Y1, Data::Characteristic::TFL, Data::Characteristic::TFNL);
				
				if (m_state.browserData.m_characteristics.size() >= 1 && globalNoisyI.size())
				{
					ImPlot::PlotLine("I", m_state.browserData.m_characteristics[0].V.data() + m_state.browserData.m_characteristics[0].dataRange.first, globalNoisyI[curr_c].first.data(), globalNoisyI[curr_c].first.size());
					ImPlot::PlotLine("I1", m_state.browserData.m_characteristics[0].V.data() + m_state.browserData.m_characteristics[0].dataRange.first, globalNoisyI[curr_c].second.data(), globalNoisyI[curr_c].second.size());
				}
			}
			ImPlot::EndPlot();
			
			ImGui::SliderInt("Char", &curr_c, 0, globalNoisyI.size() - 1);
		}
		ImGui::End();
		
		ImGui::SetNextWindowDockID(mainDockID, ImGuiCond_Once);
		if (ImGui::Begin("RT Error"))
		{
			static int curr_c = 0;
			ImVec2 s = {ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y};
			if (ImPlot::BeginPlot("RT Error", s))
			{
				ImPlot::SetupAxes("LOG(V)", "d(LOG(I))", Data::PlotData::plotSettings.xFlags, Data::PlotData::plotSettings.yFlags);
				
				// ImPlot::SetupAxisScale(ImAxis_Y1, Data::Characteristic::TFL, Data::Characteristic::TFNL);
				
				if (globalErrors.size())
				ImPlot::PlotLine("D", globalErrors[0].first.data(), globalErrors[0].second.data(), globalErrors[0].first.size());
				
				if (globalErrors.size())
				ImPlot::PlotLine("O", globalErrors[1].first.data(), globalErrors[1].second.data(), globalErrors[1].first.size());
			}
			ImPlot::EndPlot();
			ImGui::End();
			
			// ImGui::SliderInt("Char", &curr_c, 0, globalErrors.size() - 1);
		}
		*/

		// displaying MC as a separate window
		if (m_state.uiState.m_showMonteCarloInspector)
		{
			ImGui::SetNextWindowDockID(mainDockID, ImGuiCond_Once);
			if (ImGui::Begin("MC Inspector"), nullptr, ImGuiWindowFlags_NoDocking)
			{
				Views::Widgets::MonteCarloInspector(m_state.plotData);
			}
				ImGui::End();
		}
		// ImGui::ShowDemoWindow();
		if(m_state.uiState.m_showGenerator)
		{
			ImGui::SetNextWindowDockID(mainDockID, ImGuiCond_Once);
			if (ImGui::Begin("Generate"))
			{
				Views::Widgets::DataGenerator(m_state.browserData);
			}
				ImGui::End();
		}
		//ImGui::SetNextWindowDockID(mainDockID, ImGuiCond_Once);
		if (m_state.uiState.m_showArrheniusViewer)
		{
			//ImGui::SetNextWindowDockID(mainDockID, ImGuiCond_Once);
			if (ImGui::Begin("Arrhenius Plots"))
			{
				Views::Widgets::ArrheniusViewer(m_state.plotData);
			}
				ImGui::End();
		}
		}
	

	void App::update()
	{
		// if (!m_numerics && !m_dataLoader) return;
		std::scoped_lock lk{m_charMutex};

		// update the characteristic list and active characteristic
		if (m_state.plotData.active && !(m_state.plotData.active->checked))
		{
			m_state.plotData.active = nullptr;
			for (auto &ch : m_state.browserData.m_characteristics)
			{
				if (ch.checked)
				{
					m_state.plotData.active = &ch;
					break;
				}
			}
		}

		// update the parameters according to the model of the active characteristic for MC choice
		//?
	}

	void App::setUpCallbacks()
	{

		// browser callbacks

		{
			m_state.browserData.m_singleShot = [&]()
			{
				auto &genData = m_state.browserData.m_paramGenData;

				auto &data = m_state.browserData;

				auto &conf = m_state.nConfig;

				std::vector<double> V{};

				if (data.m_byStepN)
				{
					V.resize(data.m_nSteps);
					double v = data.m_voltageGenRange[0];
					double step = (data.m_voltageGenRange[1] - data.m_voltageGenRange[0]) / (data.m_nSteps - 1);
					for (auto &val : V)
					{
						val = v;
						v += step;
					}
				}
				else
				{
					double step = data.m_voltageGenStep;
					double val = data.m_voltageGenRange[0];
					while (val <= data.m_voltageGenRange[1])
					{
						V.push_back(val);
						val += step;
					}
				}

				if (V.size() == 0)
					return;

				Data::Characteristic ch{};
				ch.V = V;
				ch.I.resize(V.size());
				ch.name = "Generated";
				ch.T = data.m_genT;
				ch.modelID = data.m_genModelID;
				ch.savedModelID = data.m_genModelID;
				ch.dataRange = {0, ch.V.size() - 2};
				ch.m_tuneCallback = [&]()
				{
					const auto tmpP = ch.fittedParameters;
					ch.fittedParameters = ch.tunedParameters;
					CalculatingData cData = ch.getCalculatingData();

					m_numerics->CalculateData(cData);

					ch.fitError = m_numerics->CalculateError(cData.characteristic.currentData, ch.getEstimateInput().characteristic.currentData);
					ch.fittedParameters = tmpP;
				};

				for (auto &[id, d] : genData)
					ch.fittedParameters[id] = d.singleValue;

				ch.I.resize(V.size());
				auto cData = ch.getCalculatingData();
				m_numerics->CalculateData(cData);
				ch.I = ch.fittedI;

				static std::mt19937 rand_generator{std::random_device{}()};
				std::normal_distribution<double> distribution{0, 1};

				for (auto &i : ch.I)
				{
					double noise = (i * data.m_noise / 100.0);

					i += distribution(rand_generator) * noise;
				}

				ch.checked = true;
				ch.useBounds = true;
				ch.savedUseBounds = true;
				ch.dataRange = {0, ch.I.size() - 1};
				auto eParams = m_numerics->Estimate(ch.getEstimateInput());
				ch.savedInitialGuess = eParams;
				ch.savedUseInitial = true;
				ch.savedUseInitial = true;
				ch.fittedParameters = eParams;
				for (const auto &[k, v] : eParams)
				{
					if (k != 0 && k != 4)
					{
						ch.savedBounds[k].first = 1.0 * std::pow(10.0, std::floor(std::log10(v)) - 1);
						ch.savedBounds[k].second = 9.0 * std::pow(10.0, std::floor(std::log10(v)) + 1);
					}
					else if (k == 4)
					{
						ch.savedBounds[k].first = 1;
						ch.savedBounds[k].second = 5;
					}
					else
					{
						ch.savedBounds[k].first = 1;
						ch.savedBounds[k].second = 5;
					}
				}
				ch.savedUseBounds = true;
				ch.useBounds = true;
				m_numerics->Fit(ch.getFittingInput(), [&](ParameterMap &&output)
								{


					CalculatingData cData = ch.getCalculatingData();
					cData.parameters = output;

					m_numerics->CalculateData(cData);

					double fitError = m_numerics->CalculateError(cData.characteristic.currentData, ch.getEstimateInput().characteristic.currentData);
					ch.submitFitting(output, fitError);
					//std::scoped_lock lk{ m_charMutex };
					ch.savedUseInitial = false;
					ch.savedUseBounds = false;

					ch.bounds = ch.savedBounds;

					m_state.browserData.m_characteristics.push_back(ch);
					m_state.plotData.active = &m_state.browserData.m_characteristics.back(); });

				m_state.browserData.m_characteristics.push_back(ch);
				m_state.plotData.active = &m_state.browserData.m_characteristics.back();
			};

			m_state.browserData.m_generateCallback = [&]()
			{
				auto &genData = m_state.browserData.m_paramGenData;

				auto &data = m_state.browserData;

				auto &conf = m_state.nConfig;

				std::vector<double> V{};

				if (data.m_byStepN)
				{
					V.resize(data.m_nSteps);
					double v = data.m_voltageGenRange[0];
					double step = (data.m_voltageGenRange[1] - data.m_voltageGenRange[0]) / (data.m_nSteps - 1);
					for (auto &val : V)
					{
						val = v;
						v += step;
					}
				}
				else
				{
					double step = data.m_voltageGenStep;
					double val = data.m_voltageGenRange[0];
					while (val <= data.m_voltageGenRange[1])
					{
						V.push_back(val);
						val += step;
					}
				}

				std::vector<double> T{};

				if (data.m_tempN == 1)
					T.push_back(data.m_genT);
				else
				{
					T.resize(data.m_tempN);
					double v = data.m_tempRange[0];
					double step = (data.m_tempRange[1] - data.m_tempRange[0]) / (data.m_tempN - 1);
					for (auto &val : T)
					{
						val = v;
						v += step;
					}
				}

				std::vector<double> noise{};

				if (data.m_noiseN == 1)
					noise.push_back(data.m_noise / 100.0);
				else
				{
					noise.resize(data.m_noiseN);
					double v = data.m_noiseRange[0] / 100.0;
					double step = (data.m_noiseRange[1] / 100.0 - data.m_noiseRange[0] / 100.0) / (data.m_noiseN - 1);
					for (auto &val : noise)
					{
						val = v;
						v += step;
					}
				}

				if (V.size() == 0)
					return;

				std::unordered_map<ParameterID, std::vector<double>> params{};

				auto generateRange = [&](double start, double end, bool byNumber, size_t N, double step, Data::BrowserData::GenType type)
				{
					std::vector<double> vals{};
					if (byNumber)
					{
						vals.resize(N);
						double v = start;
						double step = (end - start) / (N - 1);
						double d_step = 10.0 / N;

						int start_pow = std::floor(std::log10(start));
						int end_pow = std::floor(std::log10(end));

						double log_step = double(end_pow - start_pow) / double(N - 1);

						switch (type)
						{
						case Data::BrowserData::GenType::Linear:
							for (auto &val : vals)
							{
								val = v;
								v += step;
							}
							break;
						case Data::BrowserData::GenType::Log:
							for (int i = 0; i < N; ++i)
							{
								double log_value = start_pow + i * log_step;
								vals[i] = std::pow(10, log_value);
							}
							break;
						case Data::BrowserData::GenType::Exponential:
							for (int i = 0; i < N; ++i)
							{
								double exponent_value = start + i * step;
								vals[i] = std::exp(exponent_value);
							}
							break;
						case Data::BrowserData::GenType::PerDecade:
							vals.resize(N * (end_pow - start_pow + 1));
							for (int i = start_pow; i <= end_pow; i++)
							{
								size_t p = 0;
								for (double j = 1.0; j < 10.0; j += d_step)
									vals[(i - start_pow) * N + p++] = j * std::pow(10, i);
							}
							break;
						}
					}
					else
					{
						double v = start;
						while (v <= end)
						{
							vals.push_back(v);
							v += step;
						}
					}
					return vals;
				};

				for (auto &[id, d] : genData)
				{
					if (d.singleShot)
						params[id] = generateRange(d.start, d.end, data.m_byStepN, d.nSteps, d.step, d.type);
					else
						params[id] = {d.singleValue};
				}

				std::vector<std::pair<ParameterMap, double>> pMaps{};
				std::vector<double> noises{};

				size_t numOfCombinations = std::accumulate(params.begin(), params.end(), 1, [](size_t acc, const auto &p)
														   { return acc * p.second.size(); });

				numOfCombinations *= T.size();
				numOfCombinations *= noise.size();

				std::map<ParameterID, size_t> indices{};

				for (const auto &[id, vals] : params)
				{
					indices[id] = 0;
				}

				indices[(*(--indices.end())).first + 1] = 0;

				ParameterID tempID = (*(--indices.end())).first;

				indices[(*(--indices.end())).first + 1] = 0;

				ParameterID noiseID = (*(--indices.end())).first;

				pMaps.resize(numOfCombinations);
				noises.resize(numOfCombinations);

				for (size_t i = 0; i < numOfCombinations; i++)
				{
					std::pair<ParameterMap, double> pMap{};
					for (auto &[id, s] : indices)
						if (id != noiseID)
							pMap.first[id == tempID ? id - 1 : id] = 0.0;

					for (auto &[id, val] : pMap.first)
						val = params[id][indices[id]];

					pMap.second = T[indices[tempID]];
					if (data.m_noiseN > 0)
						noises[i] = noise[indices[noiseID]];

					for (auto &[p, i] : indices)
					{
						i++;
						if (p == tempID && i >= T.size())
							i = 0;
						else if (p == noiseID && i >= noise.size())
							i = 0;
						else if (i >= params[p].size() && p < tempID)
							i = 0;
						else
							break;
					}

					pMaps[i] = pMap;
				}

				static std::mt19937 rand_generator{std::random_device{}()};

				for (const auto &[pMap, ns] : std::views::zip(pMaps, noises))
				{
					Data::Characteristic ch{};
					ch.V = V;
					ch.I.resize(V.size());
					ch.dataRange = {0, ch.I.size() - 1};
					ch.name = "Generated";
					ch.T = pMap.second;
					ch.modelID = data.m_genModelID;
					ch.savedModelID = data.m_genModelID;
					ch.dataRange = {0, ch.V.size() - 1};
					ch.m_tuneCallback = [&]()
					{
						ch.fittedParameters = ch.tunedParameters;
						CalculatingData cData = ch.getCalculatingData();

						m_numerics->CalculateData(cData);

						ch.fitError = m_numerics->CalculateError(cData.characteristic.currentData, ch.getEstimateInput().characteristic.currentData);
					};

					ch.fittedParameters = pMap.first;

					auto cData = ch.getCalculatingData();
					m_numerics->CalculateData(cData);
					ch.I = ch.fittedI;

					std::normal_distribution<double> distribution{0, 1};

					if (data.m_noiseN > 0)
						for (auto &i : ch.I)
						{
							double noise = (i * ns);

							i += distribution(rand_generator) * noise;
						}

					ch.checked = true;
					ch.dataRange = {0, ch.I.size() - 1};
					auto eParams = m_numerics->Estimate(ch.getEstimateInput());
					ch.savedInitialGuess = eParams;
					ch.savedUseInitial = true;
					ch.fittedParameters = eParams;
					for (const auto &[k, v] : eParams)
					{
						if (k != 1 or k != 4)
						{
							ch.savedBounds[k].first = 1.0 * std::pow(10.0, std::floor(std::log10(v)) - 1);
							ch.savedBounds[k].second = 9.0 * std::pow(10.0, std::floor(std::log10(v)) + 1);
						}
						else
						{
							ch.savedBounds[k].first = 1;
							ch.savedBounds[k].second = 5;
						}
					}
					ch.savedUseBounds = true;
					ch.useBounds = true;
					m_numerics->Fit(ch.getFittingInput(), [&](ParameterMap &&output)
									{


						CalculatingData cData = ch.getCalculatingData();
						cData.parameters = output;

						m_numerics->CalculateData(cData);

						double fitError = m_numerics->CalculateError(cData.characteristic.currentData, ch.getEstimateInput().characteristic.currentData);
						ch.submitFitting(output, fitError);
						//std::scoped_lock lk{ m_charMutex };
						ch.savedUseInitial = false;
						ch.savedUseBounds = false;

						ch.bounds = ch.savedBounds;

						m_state.browserData.m_characteristics.push_back(ch);
						m_state.plotData.active = &m_state.browserData.m_characteristics.back(); });
					m_state.browserData.m_characteristics.push_back(ch);
					m_state.plotData.active = &m_state.browserData.m_characteristics.back();
				}
			};

			auto fittingFunction = [&](Data::Characteristic &temp, const auto &numerics)
			{
				using ParametersID = JFMService::Fitters::ParameterID;
				auto eParams = numerics->Estimate(temp.getEstimateInput());
				if (temp.savedInitialGuess[ParametersID::I_sc])
					eParams[ParametersID::I_sc] = temp.savedInitialGuess[ParametersID::I_sc];

				temp.savedInitialGuess = eParams;
				temp.savedUseInitial = true;
				// fit
				temp.savedUseBounds = true;
				for (const auto &[k, v] : eParams)
				{
					if (k != 1 or k != 4)
					{
						temp.savedBounds[k].first = 1.0 * std::pow(10.0, std::floor(std::log10(v)) - 1);
						temp.savedBounds[k].second = 9.0 * std::pow(10.0, std::floor(std::log10(v)) + 1);
					}
					else
					{
						temp.savedBounds[k].first = 1;
						temp.savedBounds[k].second = 20;
					}
				}

				numerics->Fit(temp.getFittingInput(), [&](ParameterMap &&output)
							  {
								  CalculatingData cData = temp.getCalculatingData();
								  cData.parameters = output;

								  numerics->CalculateData(cData);

								  double fitError = numerics->CalculateError(cData.characteristic.currentData, temp.getEstimateInput().characteristic.currentData);
								  temp.submitFitting(output, fitError);
								  // std::scoped_lock lk{ m_charMutex };
								  temp.savedUseInitial = false;
								  temp.savedUseBounds = false;

								  temp.bounds = temp.savedBounds; });
			};

			m_state.browserData.m_loadSingleCharacteristic = [&](Data::Characteristic &temp)
			{
				using ModelID = JFMService::Fitters::JFMModelID;
				using CharacteristicType = JFMService::Fitters::CharacteristicType;
				using ParameterID = JFMService::Fitters::ParameterID;
				bool light = false;
				auto modelID = ModelID::Model4P;
				auto copiedI = temp.I;

				if (m_state.browserData.m_characteristicType == CharacteristicType::Light)
				{
					light = true;
					unsigned int index = ModelID::Model4P;
					temp.modelID = index;
					temp.savedModelID = index;

					// Check if temp.V is not empty and contains the value 0
					auto it = std::min_element(temp.V.begin(), temp.V.end(), [](double a, double b) {
						return std::abs(a) < std::abs(b);
						});
					if (it != temp.V.end())
					{
						auto minValue = std::abs(temp.I[std::distance(temp.V.begin(), it)]);
						temp.ShortCircuitCurrent = minValue;
						std::ranges::for_each(copiedI, [&](auto& item)
							{ item += minValue; });
						temp.dataRange = m_numerics->RangeData({ temp.V, copiedI });
						temp.I = copiedI;
					}
				}
				else
				{
					// Handle the case where 0 is not found in temp.V
					// You can set a default value or handle the error as needed
					temp.ShortCircuitCurrent = 0; // Default value
					temp.dataRange = m_numerics->RangeData({ temp.V, copiedI });
					temp.I = copiedI;
				}

				if (temp.forcedModelID)
				{
					temp.modelID = temp.forcedModelID;
					temp.savedModelID = temp.forcedModelID;
					fittingFunction(temp, m_numerics);
					return;
				}

				fittingFunction(temp, m_numerics);
				if (temp.fitError > 1e-4)
				{
					modelID = ModelID::Model6P;
					temp.modelID = modelID;
					temp.savedModelID = modelID;
					fittingFunction(temp, m_numerics);
				}
			};

			m_state.plotData.m_saveParametersCallback = [&](std::vector<JFMApp::Data::Characteristic> &characteristics)
			{
				std::stringstream stringStream;
				std::filesystem::path directoryPath = m_state.browserData.currentPath / "Analysis";
				std::filesystem::path filePath = directoryPath / "parameters.csv";

				stringStream << "Name\tTemperature\t";
				for (const auto &[id, name] : m_state.plotData.paramConfig->parameters)
					stringStream << name << "\t";

				bool hasLight = std::any_of(characteristics.begin(), characteristics.end(),
											[](const auto &c)
											{ return c.characteristicType == JFMService::Fitters::CharacteristicType::Light; });

				if (hasLight)
					stringStream << "Isc";

				stringStream << std::endl;
				for (const auto &characteristic : characteristics)
				{
					if (characteristic.fitted)
					{
						stringStream << characteristic.name << "\t" << characteristic.T << "\t";

						for (const auto &[id, value] : m_state.plotData.paramConfig->parameters)
						{

							auto it = characteristic.fittedParameters.find(id);
							if (it != characteristic.fittedParameters.end())
								stringStream << std::scientific << std::setprecision(3) << it->second << "\t";
							else
								stringStream << "--\t";
						}
						if (hasLight)
						{
							if (characteristic.characteristicType == JFMService::Fitters::CharacteristicType::Light)
								stringStream << std::scientific << std::setprecision(3) << characteristic.ShortCircuitCurrent;
							else
								stringStream << "--";
						}

						stringStream << std::endl; // ✅ Ensure a newline after every row
					}
				}

				if (!std::filesystem::exists(directoryPath))
					std::filesystem::create_directories(directoryPath);
				std::ofstream file(filePath, std::ios::out | std::ios::trunc);
				if (!file)
				{
					Err() << "Error: Unable to open file " << filePath << std::endl;
					return;
				}

				file << stringStream.str();
				file.close();

				Info() << "File saved successfully to: " << filePath << std::endl;
			};

			m_state.browserData.m_loadCallback = [&]()
			{
				std::vector<std::filesystem::path> paths{};

				for (const auto &[index, selected] : std::views::enumerate(std::filesystem::directory_iterator(m_state.browserData.currentPath)))
				{
					if (m_state.browserData.m_selection[index] && !selected.is_directory())
					{
						paths.push_back(selected.path());
					}
				}

				if (paths.empty())
					return;

				m_dataLoader->Load(paths, [&](std::vector<LoaderOutput> characteristics)
								   {
									for (auto &c : characteristics)
									{
										if (!c.success)
											continue;
										   // loading a characteristic
											if (c.data)
											{

												Data::Characteristic temp{*c.data};
												temp.nConfig = m_state.nConfig;
												temp.checked = true;
												temp.characteristicType = m_state.browserData.m_characteristicType;
												temp.forcedModelID = m_state.browserData.forcedModelID;
												const auto &p = std::find_if(paths.begin(), paths.end(), [&](const std::filesystem::path &path)
																			{ return path.string().contains(temp.name); });

												if (p != paths.end())
												{
													   temp.path = *p;
												}

												temp.m_tuneCallback = [&]()
												{
													// assuming the tuned parameters are copied into fitted
													CalculatingData cData = m_state.plotData.active->getCalculatingData();
													m_numerics->CalculateData(cData);
													temp.fitError = m_numerics->CalculateError(cData.characteristic.currentData, m_state.plotData.active->getEstimateInput().characteristic.currentData);
												};
												m_state.browserData.m_loadSingleCharacteristic(temp);
												m_state.browserData.m_characteristics.push_back(temp);
												m_state.plotData.active = &m_state.browserData.m_characteristics.back();
												m_state.plotData.active->tunedI = m_state.plotData.active->fittedI;
												m_state.plotData.active->tunedParameters = m_state.plotData.active->fittedParameters;
											}
									}

									   for (auto &c : characteristics)
									   {
										   if (!c.success)
											   continue;
										   // loading a monte carlo
										   //  check if the characteristic is already loaded
										   //  if not, load the characteristic
										   //
										   //  if yes, check if montecalro data is already loaded(distinguish by the fitting config)
										   //  if not, load the montecarlo data
										   //      --put montecarlo data into th echaracteristic
										   //  if yes, do nothing
										   if (c.mcData)
										   {
											   auto cc = std::find_if(m_state.browserData.m_characteristics.begin(), m_state.browserData.m_characteristics.end(), [&](Data::Characteristic &ch)
																	  { return ch.path == c.mcData->inputData.startingData.name; });

											   Data::Characteristic *ch{nullptr};

											   auto loadMC = [&]()
											   {
												   auto &mc = *c.mcData;
												   auto &cha = *ch;

												   auto mcs = std::find_if(cha.mcData.begin(), cha.mcData.end(), [&](Data::Characteristic::MCSimulation &m)
																		   { return m.fixConfig == mc.inputData.startingData.fixConfig; });

												   if (mcs == cha.mcData.end())
												   {
													   cha.submitMC(mc);
												   }
											   };

											   if (cc == m_state.browserData.m_characteristics.end())
											   {
												   {
													   m_dataLoader->Load(c.mcData->inputData.relPath, [&](LoaderOutput cdata)
																		  {
										if (cdata.success && cdata.data) {
											Data::Characteristic temp{ *cdata.data };
											temp.nConfig = m_state.nConfig;
											temp.path = c.mcData->inputData.relPath;


											temp.fittedParameters = c.mcData->inputData.trueParameters;
											temp.fixedParametersValues = c.mcData->inputData.startingData.fixConfig;
											temp.savedFixedParametersValues = temp.fixedParametersValues;
											temp.initialGuess = c.mcData->inputData.startingData.initialValues;
											temp.savedInitialGuess = temp.initialGuess;
											temp.modelID = c.mcData->inputData.startingData.initialData.modelID;
											temp.savedModelID = temp.modelID;
											temp.tunedParameters = temp.fittedParameters;
											temp.isFitted = true;
											temp.tunedParameters = temp.fittedParameters;
											temp.dataRange = m_numerics->RangeData({ temp.V, temp.I });

											auto cd = temp.getCalculatingData();
											m_numerics->CalculateData(cd);
											temp.fitError = m_numerics->CalculateError(temp.getEstimateInput().characteristic.currentData, cd.characteristic.currentData);


											std::scoped_lock lk{ m_charMutex };
											m_state.browserData.m_characteristics.push_back(temp);
											ch = &m_state.browserData.m_characteristics.back();
											loadMC();
										} });
												   }
											   }
											   else
											   {
												   ch = &(*cc);
												   loadMC();
											   }
										   }
									   } });
			};
			m_state.browserData.m_loadAllCallback = [&]()
			{
				std::vector<std::filesystem::path> paths{};

				for (const auto &[index, selected] : std::views::enumerate(std::filesystem::directory_iterator(m_state.browserData.currentPath)))
				{
					if (!selected.is_directory())
					{
						paths.push_back(selected.path());
					}
				}

				m_dataLoader->Load(paths, [&](std::vector<LoaderOutput> characteristics)
								   {
									   for (auto &c : characteristics)
									   {
										   if (!c.success)
											   continue;
										   if (c.data)
										   {
											   Data::Characteristic temp{*c.data};
											   temp.nConfig = m_state.nConfig;
											   temp.checked = true;
											   temp.characteristicType = m_state.browserData.m_characteristicType;
											   temp.forcedModelID = m_state.browserData.forcedModelID;
											   const auto &p = std::find_if(paths.begin(), paths.end(), [&](const std::filesystem::path &path)
																			{ return path.string().contains(temp.name); });

											   if (p != paths.end())
											   {
												   temp.path = *p;
											   }

											   temp.m_tuneCallback = [&]()
											   {
												   // assuming the tuned parameters are copied into fitted
												   CalculatingData cData = m_state.plotData.active->getCalculatingData();
												   m_numerics->CalculateData(cData);
												   m_state.plotData.active->fitError = m_numerics->CalculateError(cData.characteristic.currentData, m_state.plotData.active->getEstimateInput().characteristic.currentData);
											   };

											   m_state.browserData.m_loadSingleCharacteristic(temp);
											   m_state.browserData.m_characteristics.push_back(temp);
											   m_state.plotData.active = &m_state.browserData.m_characteristics.back();
											   m_state.plotData.active->tunedI = m_state.plotData.active->fittedI;
											   m_state.plotData.active->tunedParameters = m_state.plotData.active->fittedParameters;
										   }
									   } });
			};

			m_state.browserData.m_updateColorsCallback = [&]()
			{
				auto &startColor = m_state.browserData.startColor;
				auto &endColor = m_state.browserData.endColor;

				size_t cn = m_state.browserData.m_characteristics.size();

				for (const auto &[i, c] : std::views::enumerate(m_state.browserData.m_characteristics))
				{

					ImVec4 col{};
					col.w = 1.0f;

					float t = static_cast<float>(i) / static_cast<float>(cn);

					for (size_t i = 0; i < 3; i++)
					{
						auto &r = *(reinterpret_cast<float *>(&col) + i);
						auto &s = *(reinterpret_cast<float *>(&startColor) + i);
						auto &e = *(reinterpret_cast<float *>(&endColor) + i);

						r = s * (1.0f - t) + e * t;
					}
					col.w = c.color.w;
					c.color = col;
				}
			};

			m_state.browserData.m_invertSelectionCallback = [&]()
			{
				auto &active = m_state.plotData.active;
				for (auto &ch : m_state.browserData.m_characteristics)
				{
					if (active == &ch)
						active = nullptr;
					if (!ch.checked && !active)
						active = &ch;
					ch.checked = !ch.checked;
				}
			};

			m_state.browserData.m_selectAllCallback = [&]()
			{
				for (auto &ch : m_state.browserData.m_characteristics)
				{
					ch.checked = true;
				}
			};

			{
				for (auto &ch : m_state.browserData.m_characteristics)
				{
					ch.checked = false;
				}
				m_state.plotData.active = nullptr;
			};

			m_state.browserData.m_removeSelectedCallback = [&]()
			{
				// remove characteristics that are checked

				std::erase_if(m_state.browserData.m_characteristics, [&](Data::Characteristic &ch)
							  { return ch.checked; });

				m_state.plotData.active = nullptr;
			};

			m_state.browserData.m_removeUnselectedCallback = [&]()
			{
				std::erase_if(m_state.browserData.m_characteristics, [](Data::Characteristic &ch)
							  { return !ch.checked; });
			};
		}

		// plot area callbacks
		{

			m_state.plotData.m_estimateCallback = [&]()
			{
				if (!m_state.plotData.active)
					return;

				auto &active = *m_state.plotData.active;

				auto eParams = m_numerics->Estimate(active.getEstimateInput());

				active.fittedParameters = eParams;
				active.savedInitialGuess = eParams;
				active.savedBounds.clear();
				active.bounds.clear();
				for (const auto &[k, v] : eParams)
				{
					if (k != 1)
					{
						active.savedBounds[k].first = 1.0 * std::pow(10.0, std::floor(std::log10(v)) - 1);
						active.savedBounds[k].second = 9.0 * std::pow(10.0, std::floor(std::log10(v)) + 1);
					}
					else
					{
						active.savedBounds[k].first = 1;
						active.savedBounds[k].second = 5;
					}
				}
				active.bounds = active.savedBounds;
				active.savedUseBounds = true;
				active.useBounds = true;
				active.savedUseInitial = true;
			};

			m_state.plotData.m_fitCallback = [&]()
			{
				if (!m_state.plotData.active)
					return;

				auto &active = *m_state.plotData.active;

				m_numerics->Fit(active.getFittingInput(), [&](ParameterMap &&output)
								{
							if (!&active)
								return;

							active.fittedParameters = output;
							CalculatingData cData = active.getCalculatingData();

							m_numerics->CalculateData(cData);

							double fitError = m_numerics->CalculateError(cData.characteristic.currentData, active.getEstimateInput().characteristic.currentData);
							active.submitFitting(output, fitError); });
			};

			m_state.plotData.m_tuneCallback = [&]()
			{
				if (!m_state.plotData.active)
					return;

				auto &active = *m_state.plotData.active;

				auto tData = active.getTuningData();

				m_numerics->CalculateData(tData);

				active.tuneError = m_numerics->CalculateError(tData.characteristic.currentData, active.getEstimateInput().characteristic.currentData);
			};

			// m_state.plotData.m_changeModelCallback

			// Monte Carlo callbacks

			m_state.plotData.m_saveMCConfCallback = [&]()
			{
				for (auto &ch : m_state.browserData.m_characteristics)
				{
					if (!ch.checked)
						continue;
					MCOutput out{ch.getMCConfig(), {}};
					LoaderOutput lOut{};
					lOut.mcData = std::make_unique<MCOutput>(std::move(out));

					m_dataLoader->Save(ch.path.parent_path() /*lOut.mcData->inputData.relPath*/, lOut, [&](LoaderOutput out) {

					});
				}
			};

			m_state.plotData.m_performMCCallback = [&]()
			{
				if (!m_state.plotData.active)
					return;

				auto &active = *m_state.plotData.active;

				auto mcData = active.getMCConfig();

				m_numerics->Simulate(mcData, [&](MCOutput &&output)
									 {
							if (!&active)
								return;
							// std::scoped_lock lk{ *active.mcMutex };
							m_state.plotData.activeMC = nullptr;
							active.submitMC(output); });
			};

			m_state.plotData.m_performMCOnAllCallback = [&]()
			{
				for (auto &ch : m_state.browserData.m_characteristics)
				{
					if (!ch.checked)
						continue;
					ch.savedUseBounds = true;
					auto mcData = ch.getMCConfig();
					mcData.iterations = m_state.plotData.savedGlobalMCConfig.n;
					mcData.noise = m_state.plotData.savedGlobalMCConfig.sigma;

					m_numerics->Simulate(mcData, [&](MCOutput &&output)
										 { ch.submitMC(output); });
				}
			};

			m_state.plotData.m_saveMCPlot = [&](int size)
			{
				MCSave toSave{};
				auto &saved = m_state.plotData.mcPlots[size];
				toSave.x_label = saved.parameters.first;
				toSave.y_label = saved.parameters.second;
				toSave.title = saved.name;
				toSave.degreesOfFreedom = m_state.nConfig.modelParameters[saved.mc.modelID].size() - saved.mc.fixConfig.size();
				toSave.pathToSave = saved.mc.relPath / saved.name;

				for (auto &res : saved.mc.data)
					toSave.results.push_back({res.parameters, res.error});

				m_numerics->SaveMCPlot(toSave);
			};

			m_state.plotData.m_saveMCUncertainty = [&]()
			{
				std::vector<UncertaintySave> toSave{};
				for (auto &c : m_state.browserData.m_characteristics)
				{
					if (!c.checked)
						continue;
					for (auto &mc : c.mcData)
					{
						UncertaintySave u{};
						u.paramPair = c.fittedParameters;
						u.T = c.T;
						u.name = c.name;
						MCOutput out{};
						MCInput in{};
						in.trueParameters = mc.trueParameters;
						in.startingData.fixConfig = mc.fixConfig;
						out.inputData = in;

						out.mcResult.clear();
						for (auto &res : mc.data)
							out.mcResult.push_back({res.parameters, res.error});

						u.uncertainty.resize(3);
						for (size_t i = 0; i < 3; i++)
						{
							for (const auto &[key, value] : c.fittedParameters)
								u.uncertainty[i][key] = m_numerics->GetUncertainty(out, i, m_state.plotData.mcTempParams.first);
						}
						toSave.push_back(u);
					}
				}
				auto &ch = m_state.browserData.m_characteristics[0];
				std::string name = m_state.plotData.mcTempName + ".csv";
				m_numerics->SaveUncertanties(toSave, ch.path.parent_path() / name);
				// save the uncertainties
			};

			m_state.plotData.m_saveMCData = [&]()
			{
				// - TODO: If there are no MonteCarlo directory $\rightarrow$ create it
				std::filesystem::path rootPath = m_state.browserData.currentPath;
				std::filesystem::path directoryPath = rootPath / "Analysis" / "MC";
				if (!std::filesystem::exists(directoryPath))
					std::filesystem::create_directories(directoryPath);

				// - **NOTE**: In for loop for all mc data
				for (const auto &characteristic : *m_state.plotData.characteristics)
					m_state.plotData.saveOneSimulation(directoryPath, characteristic);

				// std::jthread workerThread([this, directoryPath]() {
				//	for (const auto& characteristic : *m_state.plotData.characteristics)
				//	{
				//		m_state.plotData.saveOneSimulation(directoryPath, characteristic);
				//	}
				//	});
			};
			m_state.plotData.m_plotAllMC = [&]()
			{
				using PId = JFMService::Fitters::ParameterID;
				std::vector<std::pair<PId, PId>> ids{{PId::I0, PId::A}, {PId::Rsh, PId::Rs}, {PId::Rsh, PId::Rsh2}};
				// for (auto& [mcTab, [xId, yId]] : {m_state.plotData.mcTabs,ids)
				for (int idx = 0; idx < m_state.plotData.mcTabs.size(); idx++)
				{
					auto &mcTab = m_state.plotData.mcTabs[idx];
					auto &[xId, yId] = ids[idx];
					for (auto &characteristic : *m_state.plotData.characteristics)
					{
						if (characteristic.mcData.size() == 0)
							continue;
						for (const auto &simulation : characteristic.mcData)
						{

							if (!simulation.trueParameters.contains(xId) || !simulation.trueParameters.contains(yId))
								continue;
							Data::PlotData::MCPlotsData mcData;
							mcData.mc = simulation;
							mcData.name = characteristic.buildMCPlotName(xId, yId, simulation.iterations, simulation.sigma);
							mcData.parameters = {xId, yId};
							mcData.tab = mcTab;
							mcData.id = ++m_state.plotData.mcCount;
							mcData.save = [this](int Id)
							{ m_state.plotData.m_saveMCPlot(Id); };

							mcData.remove = [this](int Id)
							{
								auto it = std::find_if(m_state.plotData.mcPlots.begin(), m_state.plotData.mcPlots.end(), [Id](const auto& mcPlot) { return mcPlot.id == Id; });
								if (it != m_state.plotData.mcPlots.end()) 
									m_state.plotData.mcPlots.erase(it);
								
							};
							m_state.plotData.mcPlots.push_back(mcData);
						}
					}
				}
			};

			m_state.plotData.m_saveAllMCPlots = [&]() 
			{
				for(int index=0;index<m_state.plotData.mcPlots.size();index++)
					m_state.plotData.m_saveMCPlot(index);
			};
			m_state.plotData.m_clearAllPlots = [&]()
			{
				m_state.plotData.mcPlots.clear();
				m_state.plotData.mcCount = 0;
			};
		}
	}
};
