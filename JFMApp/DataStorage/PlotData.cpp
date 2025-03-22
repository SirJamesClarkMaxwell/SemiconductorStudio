#include "pch.hpp"
#include "PlotData.hpp"

namespace JFMApp::Data
{
    std::unordered_map<PlotData::ArrheniusPlotType, PlotData::ArrheniusFunctionType> PlotData::ArrheniusPlotSetting::functions;
    void PlotData::submitMC(const MCOutput &mcData, Characteristic &ch)
    {
    }
    void PlotData::saveOneSimulation(const std::filesystem::path &path, const Characteristic &characteristic)
    {
        std::vector<std::array<double, 3>> chi2vec = {
            {1.0, 4.0, 9.0},
            {2.30, 6.18, 11.8},
            {3.53, 8.02, 14.2},
            {4.72, 9.72, 16.3},
            {5.89, 11.3, 18.2},
            {7.04, 12.8, 20.1}};
        auto getErrorListIndex = [&](double error, size_t dof)
        {
            size_t i = 0;
            auto &vec = chi2vec[dof - 1];

            if (error > vec[0])
                i = 1;
            if (error > vec[1])
                i = 2;

            return i;
        };
        std::vector<Characteristic::MCData> sigmaOne, sigmaTwo, sigmaThree;

        std::string name = characteristic.name;
        std::vector<std::filesystem::path> paths;
        paths.reserve(3);

        for (const auto &sigma : {"one", "two", "three"})
        {
            std::string fileName = std::format("{}_{}_Sigma.csv", name, sigma);
            paths.emplace_back(path / fileName);
        }

        std::stringstream stream;
        std::vector<std::pair<int, double>> tempParameters;

        auto sortParameters = [&](const ParameterMap &parameters)
        {
            tempParameters.clear();
            for (const auto &[id, value] : parameters)
                tempParameters.emplace_back(id, value);
            std::sort(tempParameters.begin(), tempParameters.end(),
                      [&](const auto &lhs, const auto &rhs)
                      { return lhs.first < rhs.first; });
        };

        sortParameters(characteristic.fittedParameters);
        for (const auto &[id, value] : tempParameters)
            stream << parameterIdToString((JFMService::Fitters::ParameterID)id) << "\t";
        stream << "Error\n";

        auto serializeMCPoint = [&](const Characteristic::MCData &point)
        {
            std::stringstream tmpStream;
            sortParameters(point.parameters);
            for (const auto &[id, value] : tempParameters)
                tmpStream << std::format("{:.3e}", value) << "\t";
            tmpStream << std::format("{:.3e}", point.error) << "\n";
            stream << tmpStream.str();
        };
        int fixIdsCount = 0;
        for (const auto &[id, b] : characteristic.fixedParameterIDs)
            if (b)
                fixIdsCount++;

        int dof = tempParameters.size() - fixIdsCount;
        for (const auto &d : characteristic.mcData[0].data)
        {
            int sigmaIndex = getErrorListIndex(d.error, dof);

            if (sigmaIndex == 0)
                sigmaOne.emplace_back(d);
            else if (sigmaIndex == 1)
                sigmaTwo.emplace_back(d);
            else if (sigmaIndex == 2)
                sigmaThree.emplace_back(d);
        }
        std::array<std::vector<Characteristic::MCData>, 3> points{sigmaOne, sigmaTwo, sigmaThree};
        for (int i = 0; i < 3; i++)
        {
            std::cout << "Saving to: " << path << std::endl;
            for (const auto &point : points[i])
                serializeMCPoint(point);

            if (!std::filesystem::exists(path.parent_path()))
                std::filesystem::create_directories(path.parent_path());

            std::ofstream file(paths[i], std::ios::out | std::ios::trunc);
            if (!file)
            {
                std::cerr << "Error: Could not open file: " << path << std::endl;
                continue;
            }

            file << stream.str();
            file.close();

            stream.str("");
            stream.clear();
        }
    }

    void PlotData::ArrheniusPlotSetting::updateTemperatures(const std::vector<Characteristic> &characteristics)
    {
        Temperature.clear();
        for (const auto &characteristic : characteristics)
        {
            if (not characteristic)
                continue;
            Temperature.emplace_back(inversionT ? 1000 / characteristic.T : characteristic.T);
        }
    };
    void PlotData::ArrheniusPlotData::updateStoredData(const std::vector<Characteristic> &characteristics)
    {
        int size = characteristics.size();
        yData[PlotType::Fit].clear();
        yData[PlotType::Tunned].clear();
        for (const auto &characteristic : characteristics)
        {
            if (not characteristic)
                continue;
            if (characteristic.isFitted)
                yData[PlotType::Fit].emplace_back(appliedFunction(characteristic.fittedParameters));
            if (not characteristic.toTunne)
                yData[PlotType::Tunned].emplace_back(appliedFunction(characteristic.tunedParameters));
        }
    };
    void PlotData::ArrheniusPlotData::Plot(const std::string &name,const PlotData::ArrheniusPlotSetting& settings,ImGuiID dockId)
    {
		std::array<ImVec4, 2> colors{ settings.fittedColor, settings.tunedColor };
        auto func = [&](auto dType,std::vector<float> data,auto typeName)
        {
            ImPlot::SetNextMarkerStyle(ImPlotMarker_Square, -1, colors[dType], -1.0f, colors[dType]);
            ImPlot::PlotScatter((name + "_Scatter"+ typeName).c_str(), settings.Temperature.data(), data.data(), settings.Temperature.size());
            ImPlot::SetNextLineStyle(colors[dType]);
            ImPlot::PlotLine((name + "Solid"+ typeName).c_str(), settings.Temperature.data(), data.data(), settings.Temperature.size());
        };
        //ImGui::SetNextWindowDockID(dockId);
        if(ImGui::Begin(name.c_str()))
        {
            auto flags = ImPlotAxisFlags_None;
            auto [xsize, ysize] = ImGui::GetContentRegionAvail();
            if (ImPlot::BeginPlot(name.c_str(), ImVec2(xsize, ysize), flags))
            {
                if (settings.fitPotArea)
                    flags | ImPlotAxisFlags_AutoFit;
                ImPlot::SetupAxes("1000/T [1/K]", name.c_str(),flags,flags);
                func(PlotType::Fit, yData[PlotType::Fit],"_Fit");
                func(PlotType::Tunned, yData[PlotType::Tunned],"_Tunned");
                ImPlot::EndPlot();
            }
        }
        ImGui::End();
        
    };

    PlotData::ArrheniusPlotData PlotData::AddPlot(ArrheniusPlotType type)
    {
        ArrheniusPlotData data;
        data.appliedFunction = PlotData::ArrheniusPlotSetting::functions[type];
        data.arrheniusPlotType = type;
        return data;
    }

    PlotData::ArrheniusPlotSetting::ArrheniusPlotSetting()
    {
        using ArrheniusType = PlotData::ArrheniusPlotType;
        using PId = JFMService::Fitters::ParameterID;

        functions[ArrheniusType::A_ln_I0] = [](const ParameterMap &pMap)
        { return pMap.at(PId::A) * std::log(pMap.at(PId::I0)); };
        functions[ArrheniusType::A] = [](const ParameterMap& pMap)
            { return pMap.at(PId::A); };
        functions[ArrheniusType::I0] = [](const ParameterMap& pMap)
            { return pMap.at(PId::I0); };
        functions[ArrheniusType::Rs] = [](const ParameterMap &pMap)
        { return pMap.at(PId::Rs); };
        functions[ArrheniusType::Rsh] = [](const ParameterMap &pMap)
        { return pMap.at(PId::Rsh); };
        functions[ArrheniusType::Rsh2] = [](const ParameterMap &pMap)
        { return pMap.at(PId::Rsh2); };
        functions[ArrheniusType::alpha] = [](const ParameterMap &pMap)
        { return pMap.at(PId::alpha); };
    }

}