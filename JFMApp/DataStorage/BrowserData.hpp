#pragma once
#include "pch.hpp"

#include "Characteristic.hpp"

namespace JFMApp::Data
{
	struct BrowserData
	{
		//characteristics(by value)

		//loader
		//path
		//characteristics
		//folders

		std::vector<Characteristic> m_characteristics{};
		Data::NumericsConfig* nConfig{ nullptr };
		Data::ModelID forcedModelID{JFMService::Fitters::Model4P};

		CharacteristicType m_characteristicType = JFMService::Fitters::CharacteristicType::Dark;
		CharacteristicTypeMap m_characteristicTypeMap{
			{JFMService::Fitters::CharacteristicType::Dark, "Dark" },
			{JFMService::Fitters::CharacteristicType::Light, "Light" },
		};
		void changeCharacteristicType(CharacteristicType type) { m_characteristicType = type; };

		ImVec4 endColor{ 1,224/255,0,1}, startColor{ 0.0f, 0.0f, 1.0f, 1.0f };
		

		std::filesystem::path rootPath{};
		std::filesystem::path currentPath{};

		std::vector<bool> m_selection{};

		//generation data
		ModelID m_genModelID{3};
		std::array<float, 2> m_voltageGenRange{0.01f, 1.5f};
		std::array<float, 2> m_tempRange{};
		size_t m_tempN{1};
		float m_voltageGenStep{};
		bool m_byStepN{true};
		size_t m_nSteps{300};
		float m_genT{ 100.0 };
		enum GenType { Linear = 0, Log, Exponential, PerDecade };
		std::array<std::string, 4> m_genTypes{ "Linear", "Log", "Exponential", "Per Decade" };

		size_t m_noiseN{ 1 };
		double m_noise{ 0.0 };
		std::array<float, 2> m_noiseRange{ 0.0, 0.0 };

		struct GenData
		{
			GenData(double singleVal) : singleValue{ singleVal } {}

			GenData() = default;
			GenData(const GenData&) = default;
			GenData(GenData&&) = default;
			GenData& operator=(const GenData&) = default;
			GenData& operator=(GenData&&) = default;
			~GenData() = default;

			double start{};
			double end{};
			double step{};
			size_t nSteps{};
			GenType type{};
			double singleValue{};
			bool singleShot{};
		};

		std::unordered_map<ParameterID, GenData> m_paramGenData{ {0, 2.2}, {1, 1e-13}, {2, 5e-5}, {3, 100.0f} };

		std::function<void()> m_singleShot{};
		std::function<void()> m_generateCallback{};


		std::function<void(Data::Characteristic& )> m_loadSingleCharacteristic{};
		std::function<void()> m_loadCallback{};
		std::function<void()> m_loadAllCallback{};

		std::function<void()> m_updateColorsCallback{};
		std::function<void()> m_invertSelectionCallback{};
		std::function<void()> m_selectAllCallback{};
		std::function<void()> m_unselectAllCallback{};
		std::function<void()> m_removeSelectedCallback{};
		std::function<void()> m_removeUnselectedCallback{};
		
	};
}


