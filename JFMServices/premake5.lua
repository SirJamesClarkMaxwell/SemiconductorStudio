project "JFMServices"
    kind "StaticLib"
    language "C++"
    cppdialect "C++latest"
    staticruntime(srunt)

    location ( "%{wks.location}/JFMServices" )

    files {
        '%{prj.location}/Helpers/utils.cpp',
        '%{prj.location}/Helpers/SymbolLoader/SymbolLoader.cpp',
        '%{prj.location}/Helpers/SymbolLoader/SymbolLoaderWindows.cpp',
        '%{prj.location}/Fitting/*.cpp',
        '%{prj.location}/Test.cpp',
        '%{prj.location}/Engines/MonteCarlo/*.cpp',
        '%{prj.location}/DataManegement/*.cpp',
        '%{prj.location}/Models/*.cpp',
        '%{prj.location}/Calculator/*.cpp',
        '%{prj.location}/Helpers/utils.cpp',
        '%{prj.location}/Helpers/SymbolLoader/SymbolLoader.cpp',
        '%{prj.location}/Helpers/SymbolLoader/*.h',
        '%{prj.location}/Fitting/*.hpp',
        '%{prj.location}/Test.hpp',
        '%{prj.location}/Engines/MonteCarlo/*.hpp',
        '%{prj.location}/DataManegement/*.hpp',
        '%{prj.location}/Models/*.hpp',
        '%{prj.location}/Calculator/**.h',
        '%{prj.location}/Helpers/macros.h',
        "%{wks.location}/Vendor/LambertW/*.h",
        '%{wks.location}/JunctionFitMaster/JunctionFitMaster.hpp'
    }

    includedirs {
        "%{prj.location}",
        "%{prj.location}/**",
        "%{wks.location}/JunctionFitMaster",
        "%{wks.location}/Vendor/NumericStorm/NumericStorm/headers",
        "%{wks.location}/Vendor/NumericStorm/NumericStorm/headers/**",
        "%{wks.location}/Vendor/LambertW",
        "%{wks.location}/Vendor/yaml-cpp/include",
    }

    targetdir ( "%{wks.location}/lib/%{cfg.buildcfg}/" )
    objdir ( "%{wks.location}/obj/%{cfg.buildcfg}/" )

    libdirs ( "%{wks.location}/lib/%{cfg.buildcfg}/" )

    defines
    {
        "YAML_CPP_STATIC_DEFINE"
    }

    filter { "configurations:Debug" }
        defines { "DEBUG","JFM_MODE_MULTI_CORE" }
        runtime "Debug"

    filter { "configurations:Release" }
        runtime "Release"

    filter { "system:windows" }
        ignoredefaultlibraries { "msvcrt" }
        links { "yaml-cpp","LambertW" }
