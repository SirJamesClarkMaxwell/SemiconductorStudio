#include "Dumper.hpp"
#include "IDataManager.hpp"
#include <yaml-cpp/yaml.h>

namespace JFMService
{
    std::string transferModelIDToString(ModelID id)
    {
        switch (id)
        {
        case Fitters::JFMModelID::Model4P:
            return "FourParameterModel";
        case Fitters::JFMModelID::Model6P:
            return "SixParameterModel";
        case Fitters::JFMModelID::Model4PLight:
            return "FiveParameterModel";
        case Fitters::JFMModelID::Model6PLight:
            return "SevenParameterModel";  
        default:
            return "";
        }
    };

    void serializeParameters(YAML::Emitter &emitter, const ParameterMap &parameters)
    {
        using namespace DataManagementService;
        std::vector<ParameterName> order{"A", "I0", "Rs", "Rsh", "alpha", "Rsh2"}; // NOTE this should be static and defined in the API of module!!
        for (const auto &name : order)
        {
            try
            {
                emitter << parameters.at(parameterNameToID(name));
            }
            catch (const std::exception &)
            {
                continue;
            }
        }
    };
    void MonteCarloResultDumper::Save(const path &path, const LoaderOutput &toSave, const Callback &callback)
    {
        MCOutput data = *(toSave).mcData;
        YAML::Emitter out;
        // out<< YAML::BeginMap << YAML::Key << "general" << YAML::Value;
        emitConfig(out, data.inputData);
        emitMCSimulation(out, data.mcResult);
        // out << YAML::EndMap;
        std::ofstream output(path);
        output << out.c_str();
        // LoaderOutput toCallback(toSave);
        // if (callback)
        //     callback(toCallback);
    };

    void MonteCarloResultDumper::Save(const std::vector<path> &path, const std::vector<LoaderOutput> &toSave, const VectorCallback &callback)
    {
    }
    void MonteCarloResultDumper::emitConfig(YAML::Emitter &emitter, const MCInput &toEmit)
    {

        emitter << YAML::BeginMap;
        emitter << YAML::Key << "config" << YAML::Value;
        emitter << YAML::BeginMap;

        emitter << YAML::Key << "model" << YAML::Value << transferModelIDToString(toEmit.startingData.initialData.modelID);
        emitter << YAML::Key << "relativePath" << YAML::Value << toEmit.relPath.string();
        emitter << YAML::Key << "CharacteristicName" << YAML::Value << std::string(toEmit.startingData.name);
        emitter << YAML::Key << "idealParameters" << YAML::Value << YAML::BeginSeq;
        serializeParameters(emitter, toEmit.trueParameters);
        emitter << YAML::EndSeq;
        emitter << YAML::Key << "Temperature" << YAML::Value << toEmit.startingData.initialData.additionalParameters.at(Fitters::AdditionalParametersID::Temperature);
        emitter << YAML::Key << "numberOfIterations" << YAML::Value << toEmit.iterations;
        emitter << YAML::Key << "fixingConfiguration" << YAML::BeginSeq;
        for (const auto [id, val] : toEmit.startingData.fixConfig)
            emitter << parameterIdToString((Fitters::ParameterID)id);
        emitter << YAML::EndSeq;
        emitter << YAML::Key << "fixedValues" << YAML::BeginSeq;
        for (const auto [id, val] : toEmit.startingData.fixConfig)
            emitter << val;
        emitter << YAML::EndSeq;
        emitter << YAML::Key << "order" << YAML::BeginSeq;
        for (const auto item : {"A", "I0", "Rs", "Rsh", "alpha", "Rsh2"})
        {
            auto it = toEmit.startingData.fixConfig.find(DataManagementService::parameterNameToID(item));
            if (it != toEmit.startingData.fixConfig.end())
                emitter << item;
        }
        emitter << YAML::EndSeq;
        emitter << YAML::Key << "noise" << YAML::Value << toEmit.noise;
        emitter << YAML::Key << "CharacteristicLowRange" << YAML::Value << toEmit.characteristicBounds.first;
        emitter << YAML::Key << "CharacteristicUpRange" << YAML::Value << toEmit.characteristicBounds.second;
        emitter << YAML::EndMap;
        // emitter << YAML::EndMap;
    }
    void MonteCarloResultDumper::emitMCSimulation(YAML::Emitter &emitter, const std::vector<MCResult> &simulation)
    {
        // data
        emitter << YAML::Key << "data" << YAML::Value;
        emitter << YAML::BeginSeq;
        for (const auto &result : simulation)
        {
            // result
            emitter << YAML::BeginMap;

            // parameters
            emitter << YAML::Key << "result" << YAML::Value << YAML::BeginMap;
            emitter << YAML::Key << "parameters";
            emitter << YAML::Value << YAML::BeginSeq;
            serializeParameters(emitter, result.foundParameters);
            emitter << YAML::EndSeq; // end of parameters sequence

            // error
            emitter << YAML::Key << "error" << YAML::Value << result.error;
            emitter << YAML::EndMap; // end of result map

            emitter << YAML::EndMap; // end of each item in the sequence
        }
        emitter << YAML::EndSeq; // end of data sequence
    }
}