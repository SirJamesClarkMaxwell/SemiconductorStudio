JFMServices_sources = [
'./Helpers/utils.cpp',
'./Helpers/SymbolLoader/SymbolLoader.cpp',
'./Helpers/SymbolLoader/SymbolLoaderNix.cpp',
'./Helpers/SymbolLoader/SymbolLoaderWindows.cpp',
'./Fitting/PreFitter.cpp',
'./Fitting/JFMFitter.cpp',
'./Fitting/JFMFitting.cpp',
'./Test.cpp',
'./Engines/MonteCarlo/JFMMonteCarlo.cpp',
'./Engines/MonteCarlo/MonteCarloEngine.cpp',
'./DataManegement/Dumper.cpp',
'./DataManegement/DataManager.cpp',
'./DataManegement/Loader.cpp',
'./DataManegement/IDataManager.cpp',
'./Models/JFMAdditionalParameters.cpp',
'./Models/JFMErrorModel.cpp',
'./Models/JFMModels.cpp',
'./Models/CalculateData.cpp',
'./Calculator/CalculatorCpu.cpp',
]

if get_option('mode').contains('gpu')
    JFMServices_sources += 'Calculator/CalculatorGpu.cpp'
endif

JFMServices_includes = [
    'DataManegement',
    'Fitting',
    'Models',
    'Engines',
    'Engines/MonteCarlo',
    'Helpers',
    './'
]
