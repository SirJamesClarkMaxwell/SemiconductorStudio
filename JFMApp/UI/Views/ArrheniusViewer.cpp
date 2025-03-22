#include "pch.hpp"
#include "Widgets.hpp"
#include "PlotData.hpp" // Include the header file for PlotData

namespace JFMApp::Views
{
    void Widgets::ArrheniusViewer(Data::PlotData &data)
    {
        using PlotType = Data::PlotData::ArrheniusPlotType; // Fix the namespace to Data
        using ArrheniusPlotSetting = Data::PlotData::ArrheniusPlotSetting;
        if (ImGui::Button("Invert Temperature"))
            data.m_arrheniusPlotSettings.inversionT = !data.m_arrheniusPlotSettings.inversionT;
        ImGui::SameLine();

        if (ImGui::Button("Fit Plots"))
            data.m_arrheniusPlotSettings.fitPotArea = !data.m_arrheniusPlotSettings.fitPotArea;

        ImGui::SameLine();
        PlotType &type = data.m_arrheniusPlotSettings.currentPlotType;
        ImGui::PushItemWidth(100);
        if (ImGui::BeginCombo("Next-Plot", ArrheniusPlotSetting::convertTypeToString(type).c_str()))
        {
            for (int id = 0; id <= PlotType::None; id++)
            {
                if (ImGui::Selectable(ArrheniusPlotSetting::convertTypeToString(static_cast<PlotType>(id)).c_str()))
                    type = static_cast<PlotType>(id);
            }
            ImGui::EndCombo();
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("Add Plot"))
            data.m_arrheniusPlotData.emplace_back(data.AddPlot(type));
        ImGui::SameLine();
        ImGui::ColorEdit4("Fitted Color", (float *)&data.m_arrheniusPlotSettings.fittedColor, ImGuiColorEditFlags_NoInputs);
        ImGui::SameLine();
        ImGui::ColorEdit4("Tunned Color", (float *)&data.m_arrheniusPlotSettings.tunedColor, ImGuiColorEditFlags_NoInputs);
        if(ImGui::BeginChild("DockSpace", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar))
        {
            auto ArrheniusDockId = ImGui::DockSpace(ImGui::GetID("DockSpace"));

            // Plot Data

            auto settings = data.m_arrheniusPlotSettings;
            settings.updateTemperatures(*data.characteristics);
            for (auto& item : data.m_arrheniusPlotData)
            {
                item.updateStoredData(*data.characteristics);
                std::string name = ArrheniusPlotSetting::convertTypeToString(item.arrheniusPlotType);
                //ImGui::SetNextWindowDockID(ArrheniusDockId);
                item.Plot(name, settings);

            }
            ImGui::EndChild();
        }
    }
};
