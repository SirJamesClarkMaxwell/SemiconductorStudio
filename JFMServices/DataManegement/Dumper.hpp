#pragma once
#include "IDataManager.hpp"
#include "../Models/CalculateData.hpp"
#include "../Fitting/JFMFitter.hpp"
#include "../Fitting/JFMIFitting.hpp"
#include <yaml-cpp/yaml.h>
namespace JFMService
{
    using namespace FittingService;
    using namespace DataManagementService;
    using path = std::filesystem::path;
    class Dumper
    {
    public:
        using Callback = std::function<void(LoaderOutput)>;
        using VectorCallback = std::function<void(std::vector<LoaderOutput>)>;

        virtual ~Dumper() = default;
        virtual void Save(const path &path, const LoaderOutput &toSave, const Callback &callback) = 0;
        virtual void Save(const std::vector<path> &path, const std::vector<LoaderOutput> &toSave, const VectorCallback &callback) = 0;
    };
    class CharacteristicDumper : public Dumper
    {
        virtual void Save(const path &path, const LoaderOutput &toSave, const Callback &callback) {};
        virtual void Save(const std::vector<path> &path, const std::vector<LoaderOutput> &toSave, const VectorCallback &callback) {};
    };
    class MonteCarloResultDumper : public Dumper
    {
    public:
        virtual void Save(const path &path, const LoaderOutput &toSave, const Callback &callback);
        virtual void Save(const std::vector<path> &path, const std::vector<LoaderOutput> &toSave, const VectorCallback &callback);

    private:
        void emitConfig(YAML::Emitter &emitter, const MCInput &toEmit);
        void emitMCSimulation(YAML::Emitter &emitter, const std::vector<MCResult> &simulation);
    };
}
