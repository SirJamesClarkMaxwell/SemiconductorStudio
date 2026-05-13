project "JunctionFitMaster"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++latest"
    staticruntime(srunt)

    location("%{wks.location}/JunctionFitMaster")

    files {
        "%{prj.location}/**.cpp",
        "%{prj.location}/**.hpp",
    }


    includedirs {
        "%{prj.location}/**",
        "%{wks.location}/JFMApp",
        "%{wks.location}/JFMApp/**",
        "%{wks.location}/JFMServices/**",
        "%{wks.location}/JFMServices",

        "%{wks.location}/JFMServices",
        "%{wks.location}/Vendor/NumericStorm/NumericStorm/headers",
        "%{wks.location}/Vendor/NumericStorm/NumericStorm/headers/**",
        "%{wks.location}/Vendor/imgui",
        "%{wks.location}/Vendor/imgui/backends",
        "%{wks.location}/Vendor/imgui/misc/cpp",
        "%{wks.location}/Vendor/implot",
        "%{wks.location}/Vendor/imgui/examples/libs/glfw/include",
        "%{wks.location}/Vendor/yaml-cpp/include",
        "%{wks.location}/Vendor/LambertW"
    }
    defines
    {
             "YAML_CPP_STATIC_DEFINE"
    }

    targetdir("%{wks.location}/bin/%{cfg.buildcfg}/")
    objdir("%{wks.location}/obj/%{cfg.buildcfg}/")

    libdirs("%{wks.location}/lib/%{cfg.buildcfg}/")

    filter { "configurations:Debug" }
        defines { "DEBUG" }
        runtime "Debug"

    filter { "configurations:Release" }
        runtime "Release"

    filter { "system:windows", "configurations:Debug" }
        ignoredefaultlibraries { "msvcrt" }

    filter { "system:windows" }
        -- links { "JFMApp", "JFMServices" }
        links { "JFMApp", "JFMServices" }


    
