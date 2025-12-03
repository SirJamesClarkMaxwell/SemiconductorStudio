#pragma once
#include "pch.hpp"
#include "utils.hpp"
#include "IDataManager.hpp"
#include "../Models/CalculateData.hpp"
#include <yaml-cpp/yaml.h>
namespace JFMService
{
    using namespace DataManagementService;
    using namespace FittingService;
    class Loader
    {
    public:
        virtual ~Loader() = default;
        virtual bool CheckExtentionCompatibility(const std::filesystem::path &path) = 0;
        virtual LoaderOutput Load(const std::filesystem::path &path) = 0;
    };

    class CharacteristicLoader : public Loader
    {
        using loaderFunction = std::function<std::vector<std::vector<double>>(std::string &content)>;

    public:
        CharacteristicLoader();
        virtual LoaderOutput Load(const std::filesystem::path &path) override;
        virtual bool CheckExtentionCompatibility(const std::filesystem::path &path) override;

    private:
        std::unordered_map<std::string, loaderFunction> loaders;

    private:
        std::vector<std::vector<double>> loadDatContent(std::string &content);
        void readFileContent(const std::filesystem::path &path, std::string &content);
        void parseContent(const std::filesystem::path &path, std::string &content, LoaderOutput &output);
        double readTemperature(const std::string &name);
        std::string readName(const std::filesystem::path &path);
    };

    class MonteCarloResultLoader : public Loader
    {
    public:
        MonteCarloResultLoader() = default;
        virtual LoaderOutput Load(const std::filesystem::path &path) override;
        virtual bool CheckExtentionCompatibility(const std::filesystem::path &path) override;

    private:
        void loadConfig(const YAML::Node &yamlObject, MCInput &destination);
        std::vector<MCResult> loadFittingResults(const YAML::Node &node, std::vector<ParameterName> &order, size_t size);
        void transferParameters(ParameterMap &destination, const std::vector<ParameterName> names, const std::vector<double> values);
    };
}
