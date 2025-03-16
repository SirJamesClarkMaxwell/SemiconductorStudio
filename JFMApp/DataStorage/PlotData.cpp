#include "pch.hpp"
#include "PlotData.hpp"

namespace JFMApp::Data
{
	void PlotData::submitMC(const MCOutput& mcData, Characteristic& ch) {
		

	}
	void PlotData::saveOneSimulation(const std::filesystem::path& path, const Characteristic& characteristic)
	{
        std::vector<std::array<double, 3>> chi2vec = {
        {1.0, 4.0, 9.0},
        {2.30, 6.18, 11.8},
        {3.53, 8.02, 14.2},
        {4.72, 9.72, 16.3},
        {5.89, 11.3, 18.2},
        {7.04, 12.8, 20.1}
            };
        auto getErrorListIndex = [&](double error, size_t dof)
        {
            size_t i = 0;
            auto& vec = chi2vec[dof - 1];

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

        for (const auto& sigma : { "one", "two", "three" }) {
            std::string fileName = std::format("{}_{}_Sigma.csv", name, sigma);
            paths.emplace_back(path /fileName);
        }


        std::stringstream stream;
        std::vector<std::pair<int, double>> tempParameters;

        auto sortParameters = [&](const ParameterMap& parameters)
            {
                tempParameters.clear(); 
                for (const auto& [id, value] : parameters)
                    tempParameters.emplace_back(id, value);
                std::sort(tempParameters.begin(), tempParameters.end(),
                    [&](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });
            };

        sortParameters(characteristic.fittedParameters);
        for (const auto& [id, value] : tempParameters)  
            stream << parameterIdToString((JFMService::Fitters::ParameterID)id) << "\t";
        stream << "Error\n";

        auto serializeMCPoint = [&](const Characteristic::MCData& point)
            {
                std::stringstream tmpStream; 
                sortParameters(point.parameters);
                for (const auto& [id, value] : tempParameters)
                    tmpStream << std::format("{:.3e}", value) << "\t";
                tmpStream << std::format("{:.3e}", point.error) << "\n";
                stream << tmpStream.str();  
            };
        int fixIdsCount = 0;
        for (const auto& [id, b] : characteristic.fixedParameterIDs)
            if (b)
                fixIdsCount++;

        int dof = tempParameters.size() - fixIdsCount;
        for (const auto& d : characteristic.mcData[0].data)
        {
            int sigmaIndex = getErrorListIndex(d.error, dof);

            if (sigmaIndex == 0)
                sigmaOne.emplace_back(d);
            else if (sigmaIndex == 1)
                sigmaTwo.emplace_back(d);
            else if (sigmaIndex == 2)
                sigmaThree.emplace_back(d);
        }
        std::array<std::vector<Characteristic::MCData>, 3> points{ sigmaOne,sigmaTwo,sigmaThree };
        for (int i = 0; i < 3;i++)
        {
            std::cout << "Saving to: " << path << std::endl; 
            for (const auto& point : points[i])
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
    
}