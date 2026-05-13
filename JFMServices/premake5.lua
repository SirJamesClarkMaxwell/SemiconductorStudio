project "JFMServices"
    kind "StaticLib"
    language "C++"
    cppdialect "C++latest"
    staticruntime(srunt)

    location ( "%{wks.location}/JFMServices" )

    files {
        "%{prj.location}/**.cpp",
        "%{prj.location}/**.hpp",
        "%{wks.location}/Vendor/LambertW/*.h",
    }

    includedirs {
        "%{prj.location}",
        "%{prj.location}/**",
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
        defines { "DEBUG" }
        runtime "Debug"

    filter { "configurations:Release" }
        runtime "Release"

    filter { "system:windows", "configurations:Debug" }
        ignoredefaultlibraries { "msvcrt" }

    filter { "system:windows" }
        links { "yaml-cpp","LambertW" }
