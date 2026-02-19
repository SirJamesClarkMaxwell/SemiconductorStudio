
#include "tests.hpp"
#include "yaml-cpp/yaml.h"
// #include "DataManager.hpp"
void Tests::test()
{
    // testDataManager();
    //  testModel();
    //  testYAML();
    //  testFitting();
    // testAutoRange();
    testPlots();
}

void Tests::testDataManager()
{
    std::cout << "test Data Manager" << std::endl;
    testLoadingCharacteristics();
    testYAMLDumping();
    testYAMLLoading();
}

void Tests::testLoadingCharacteristics()
{
    std::cout << "----------------------" << std::endl;
    std::cout << "Loading Characteristics" << std::endl;
    DataManager manager;
    LoaderOutput loaded;
    manager.Load("ivd_HZB25_T181_L0.dat", [&](LoaderOutput output)
                 { loaded = std::move(output); });
    double T = (*loaded.data).Temperature;
    std::string name = (*loaded.data).Name;
    std::vector<double> V = (*loaded.data)[CharacteristicData::Voltage];
}

void Tests::testModel()
{
    NumericStorm::Fitting::Parameters<4> parameters{{1, 1e-8, 5e-5, 5e5}};
    JFMParameters fixedValues{2, 1e-9, 5e-6, 5e6};
    JFMAdditionalParameters additional(330.0, fixedValues, FixingConfiguration::A | FixingConfiguration::I0);
    std::vector<double> voltages;
    utils::generateVectorAtGivenRanges(voltages, 0.1, 3, 0.1);
    std::vector<double> currents(voltages);
    NumericStorm::Fitting::Data data(2);
    data[0] = voltages;
    data[1] = currents;
    FourParameterModel model;
};

void Tests::testYAMLLoading()
{

    std::cout << "----------------------" << std::endl;
    std::cout << "Loading YAML config " << std::endl;
    DataManager manager;
    LoaderOutput loaded;
    manager.Load("testLoad.yaml", [&](LoaderOutput output)
                 { loaded = std::move(output); });
    MCOutput out = *(loaded.mcData);
};
void Tests::testYAMLDumping()
{
    MCOutput mcOutput;
    MCInput mcInOutput;
    FittingInput fittingInputData;
    FittingService::EstimateInput estimateData;

    estimateData.modelID = JFMModelID::Model4P;
    AdditionalParameterMap additional;
    additional[AdditionalParametersID::Temperature] = 330;
    estimateData.additionalParameters = additional;

    fittingInputData.initialData = estimateData;
    fittingInputData.name = "ExampleName";
    ParameterMap fixingMap;
    fixingMap[Fitters::ParameterID::Rs] = 5e-5;
    fixingMap[Fitters::ParameterID::Rsh] = 5e5;

    ParameterMap initialMap;
    initialMap[Fitters::ParameterID::A] = 1;
    initialMap[Fitters::ParameterID::I0] = 5e-8;
    initialMap[Fitters::ParameterID::Rs] = 5e-5;
    initialMap[Fitters::ParameterID::Rsh] = 5e5;

    ParameterMap trueMap;
    trueMap[Fitters::ParameterID::A] = 1.1;
    trueMap[Fitters::ParameterID::I0] = 5.5e-8;
    trueMap[Fitters::ParameterID::Rs] = 5.5e-5;
    trueMap[Fitters::ParameterID::Rsh] = 5.5e5;

    fittingInputData.fixConfig = fixingMap;
    fittingInputData.initialValues = initialMap;

    mcInOutput.startingData = fittingInputData;
    mcInOutput.relPath = "MCResult";

    mcInOutput.iterations = 1e4;
    mcInOutput.noise = 1;
    mcInOutput.trueParameters = trueMap;
    mcInOutput.characteristicBounds = {5, 250};
    std::vector<MCResult> results;
    MCResult onePoint;
    std::vector<double> mcResult{2, 6e-8, 5e-6, 5e6};
    for (int i = 0; i < 3; i++)
    {
        for (const auto &[i, val] : std::views::enumerate(mcResult))
        {
            onePoint.foundParameters[(Fitters::ParameterID)i] = val;
        }
        onePoint.error = i;
        results.push_back(onePoint);
    }
    mcOutput.inputData = mcInOutput;
    mcOutput.mcResult = results;

    LoaderOutput output;
    output.mcData = std::make_unique<MCOutput>(mcOutput);
    DataManager manager;
    manager.Save("testDump.yaml", output, nullptr);
    // std::cin.get();
}
void Tests::testFitting()
{
    std::cout << std::endl
              << std::endl
              << "----------------------" << std::endl;
    std::cout << "Testing Fitting" << std::endl;

    // testFourParameterModel();
    testSixParameterModel();
    // std::cin.get();
}
void Tests::testSixParameterModel()
{
    CalculatingData plotData;
    DataCalculator calculator;
    std::vector<double> V, I;
    utils::generateVectorAtGivenRanges(V, 0.01, 1, 0.001);
    I = V;
    {

        PlotData characteristic;
        ParameterMap parameters;
        AdditionalParameterMap additional;
        characteristic.voltageData = {V.begin(), V.size()};
        characteristic.currentData = {I.begin(), I.size()};
        std::array<double, 6> p{1.65, 4e-12, 15, 2.5e4, 3.5, 6e3};
        for (const auto &[id, value] : std::views::enumerate(p))
            parameters[(Fitters::ParameterID)id] = value;
        additional[Fitters::AdditionalParametersID::Temperature] = 220;

        plotData.characteristic = characteristic;
        plotData.additionalParameters = additional;
        plotData.parameters = parameters;
        plotData.modelID = JFMModelID::Model6P;
        calculator.CalculateData(plotData);
    }

    EstimateInput estimateInput = createEstimateInput(plotData);
    //    FittingInput input = createFittingInput(estimateInput);
    FittingInput input;
    input.initialData = estimateInput;
    std::array<double, 6> p{1.8, 7e-12, 30, 9e4, 4, 7e3};
    ParameterMap parameters;
    for (const auto &[id, value] : std::views::enumerate(p))
        parameters[(Fitters::ParameterID)id] = value;

    ParamBounds bounds;
    std::array<double, 6> minBound{0.5, 1e-12, 10, 1e3, 1, 1e3};
    std::array<double, 6> maxBound{3, 9e-12, 50, 9e5, 6, 9e3};
    for (const auto &[id, min, max] : std::views::zip(std::ranges::iota_view(0, 7), minBound, maxBound))
        bounds[(Fitters::ParameterID)id] = {min, max};
    input.bounds = bounds;
    input.useBounds = true;
    input.initialValues = parameters;

    Fitters::Fitter fitter;
    ParameterMap out;
    fitter.Fit(input, [&](ParameterMap &&result)
               { out = std::move(result); });
}
void Tests::testFourParameterModel()
{
    CalculatingData plotData;
    DataCalculator calculator;
    std::vector<double> V, I;
    utils::generateVectorAtGivenRanges(V, 0.01, 3, 0.1);
    I = V;
    {

        PlotData characteristic;
        ParameterMap parameters;
        AdditionalParameterMap additional;
        characteristic.voltageData = {V.begin(), V.size()};
        characteristic.currentData = {I.begin(), I.size()};
        std::array<double, 4> p{1, 5e-8, 5e-5, 5e8};
        for (const auto &[id, value] : std::views::enumerate(p))
            parameters[(Fitters::ParameterID)id] = value;
        additional[Fitters::AdditionalParametersID::Temperature] = 330;

        plotData.characteristic = characteristic;
        plotData.additionalParameters = additional;
        plotData.parameters = parameters;
        plotData.modelID = JFMModelID::Model4P;
        calculator.CalculateData(plotData);
    }

    EstimateInput estimateInput = createEstimateInput(plotData);
    FittingInput input = createFittingInput(estimateInput);

    Fitters::Fitter fitter;
    ParameterMap out;
    fitter.Fit(input, [&](ParameterMap &&result)
               { out = std::move(result); });
};

void Tests::createData(CalculatingData &data)
{
    DataCalculator calculator;
    std::vector<double> V, I;
    utils::generateVectorAtGivenRanges(V, 0.01, 3, 0.1);
    I = V;

    PlotData characteristic;
    ParameterMap parameters;
    AdditionalParameterMap additional;
    characteristic.voltageData = {V.begin(), V.size()};
    characteristic.currentData = {I.begin(), I.size()};
    std::array<double, 4> p{10, 5e-8, 9e-5, 9e5};
    for (const auto &[id, value] : std::views::enumerate(p))
        parameters[(Fitters::ParameterID)id] = value;
    additional[Fitters::AdditionalParametersID::Temperature] = 330;

    data.characteristic = characteristic;
    data.additionalParameters = additional;
    data.parameters = parameters;
    data.modelID = JFMModelID::Model4P;
    calculator.CalculateData(data);
}

JFMService::FittingService::EstimateInput Tests::createEstimateInput(const CalculatingData &calculatingDataInput)
{
    EstimateInput toReturn;
    toReturn.additionalParameters = calculatingDataInput.additionalParameters;
    toReturn.modelID = calculatingDataInput.modelID;
    toReturn.characteristic = calculatingDataInput.characteristic;
    return toReturn;
}

JFMService::FittingService::FittingInput Tests::createFittingInput(const JFMService::FittingService::EstimateInput &estimateInput)
{
    FittingInput toReturn;
    toReturn.initialData = estimateInput;
    // std::array<double, 4> p{1.1, 6e-8, 6e-5, 6e5};
    std::array<double, 4> p{5, 5e-5, 5e-2, 5e5};
    ParameterMap parameters;
    for (const auto &[id, value] : std::views::enumerate(p))
        parameters[(Fitters::ParameterID)id] = value;

    ParamBounds bounds;
    std::array<double, 4> minBound{0.5, 1e-12, 1e-9, 1e2};
    std::array<double, 4> maxBound{20, 9e-3, 9e-3, 9e9};
    for (const auto &[id, min, max] : std::views::zip(std::ranges::iota_view(0, 5), minBound, maxBound))
        bounds[(Fitters::ParameterID)id] = {min, max};
    toReturn.bounds = bounds;
    toReturn.useBounds = true;
    toReturn.initialValues = parameters;

    return toReturn;
};
void Tests::testAutoRange()
{

    std::filesystem::path path = "ivd_HZB25_T181_L0.dat";
    using namespace JFMService;
    using namespace DataManagementService;
    using namespace JFMService::FittingService;
    DataManager manager;
    LoaderOutput output;

    manager.Load(path, [&](LoaderOutput result)
                 { output = std::move(result); });
    FourParameterModelPreFit prefitter;
    CharacteristicData characteristic = *output.data;
    using names = CharacteristicData::DataNames;
    std::span<double> V{characteristic[names::Voltage].begin(), characteristic[names::Voltage].end()}, I{characteristic[names::Current].begin(), characteristic[names::Current].end()};
    JFMService::FittingService::PlotData data;
    data.voltageData = V;
    data.currentData = I;
    std::pair<double, double> range = prefitter.rangeData(data);
    std::cout << "lower range: " << range.first << " upper range: " << range.second;
}
void Tests::testPlots()
{
    double AStart{1.0}, I0Start{1e-8}, Rs{5e-5}, Rsh{5e5};
    double AEnd{2}, I0End{9e-8};
    size_t numberOfSteps{500};
    double AStep{(AEnd - AStart) / numberOfSteps}, I0Step{(I0End - I0Start) / numberOfSteps};
    ParameterMap tmpMap;
    std::vector<MCResult> results;
    tmpMap[Fitters::Rs] = Rs;
    tmpMap[Fitters::Rsh] = Rsh;
    for (size_t i = 0; i < numberOfSteps; i++)
    {
        MCResult tmpResult;
        tmpMap[Fitters::A] = AStart + i * AStep;
        tmpMap[Fitters::I0] = I0Start + i * I0Step;
        tmpResult.foundParameters = tmpMap;
        tmpResult.error = i / 100.0;
        results.push_back(tmpResult);
    }
    FittingService::Fitting interfaceObject;
    FittingService::MCSave toSave;
    toSave.degreesOfFreedom = 4;
    toSave.x_label = Fitters::A;
    toSave.y_label = Fitters::I0;
    toSave.results = results;
    toSave.title = "Example Title";
    toSave.pathToSave = "D:/STUDIA/Semiconductor-Studio/SemiconductorStudio/bin/testsToSave.txt";
    interfaceObject.SaveMCPlot(toSave);
};
