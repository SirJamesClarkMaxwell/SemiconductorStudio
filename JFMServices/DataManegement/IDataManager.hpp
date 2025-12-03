#pragma once
#include "pch.hpp"
#include "../Fitting/JFMIFitting.hpp"
#include "../Fitting/JFMFitter.hpp"
namespace JFMService::DataManagementService
{
    struct CharacteristicData
    {
    public:
        enum DataNames : unsigned int // todo move this into app site
        {
            Voltage = 0,
            Current,
            DensityCurrent
        };

        CharacteristicData() = default;
        CharacteristicData(std::vector<std::vector<double>> &data, double temperature, const std::string &name);

        std::vector<std::vector<double>> Data;
        double Temperature{-1};
        std::string Name;

        std::vector<double> &operator[](unsigned int name) { return Data.at(name); }
    };
    struct LoaderOutput
    {

        std::unique_ptr<CharacteristicData> data{nullptr};
        std::unique_ptr<FittingService::MCOutput> mcData{nullptr};
        bool success{};
    };

    class IDataManager
    {
    public:
        using Callback = std::function<void(LoaderOutput)>;
        using VectorCallback = std::function<void(std::vector<LoaderOutput>)>; // todo change this type to be more suitable
        using path = std::filesystem::path;

        virtual void Load(const path &path, const Callback &callback) = 0;
        virtual void Load(const std::vector<path> &paths, const VectorCallback &callbacks) = 0;

        virtual void Save(const path &path, const LoaderOutput &input, const Callback &callback) = 0;
        virtual void Save(const std::vector<path> &paths, const std::vector<LoaderOutput> &input, const VectorCallback &callbacks) = 0;

        virtual void MoveInto(path &currentPath, const std::string &destination) = 0;
        virtual void MoveBack(path &currentPath) = 0;

        virtual std::vector<std::string> GetDirectories(const path &path) = 0;
        virtual std::vector<std::string> GetFiles(const path &path) = 0;
    };
    static Fitters::ParameterID parameterNameToID(const std::string &name)
    {
        if (name == "A")
            return Fitters::ParameterID::A;
        if (name == "I0")
            return Fitters::ParameterID::I0;
        if (name == "Rs")
            return Fitters::ParameterID::Rs;
        if (name == "Rsh")
            return Fitters::ParameterID::Rsh;
        if (name == "alpha")
            return Fitters::ParameterID::alpha;
        if (name == "Rsh2")
            return Fitters::ParameterID::Rsh2;
        Unreachable();
    };
    static std::string parameterIdToString(Fitters::ParameterID id)
    {
        switch (id)
        {
        case Fitters::ParameterID::A:
            return "A";
        case Fitters::ParameterID::I0:
            return "I0";
        case Fitters::ParameterID::Rs:
            return "Rs";
        case Fitters::ParameterID::Rsh:
            return "Rsh";
        case Fitters::ParameterID::alpha:
            return "alpha";
        case Fitters::ParameterID::Rsh2:
            return "Rsh2";
        default:
            return "";
        }
    };
}
