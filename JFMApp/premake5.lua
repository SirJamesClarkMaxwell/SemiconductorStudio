project "JFMApp"
    kind "StaticLib"
    language "C++"
    cppdialect "C++latest"
    staticruntime(srunt)

    location ( "%{wks.location}/JFMApp" )

    files {
        "%{prj.location}/**.cpp",
        "%{prj.location}/**.hpp",
        "%{prj.location}/*.cpp",
        "%{prj.location}/*.hpp"
    }

    includedirs {
        "%{prj.location}",
        "%{prj.location}/**",
        "%{wks.location}/JFMServices",
        "%{wks.location}/JFMServices/**",
        "%{wks.location}/Vendor/imgui",
        "%{wks.location}/Vendor/imgui/backends",
        "%{wks.location}/Vendor/imgui/misc/cpp",
        "%{wks.location}/Vendor/implot",
        "%{wks.location}/Vendor/imgui/examples/libs/glfw/include"
    }

    targetdir ( "%{wks.location}/lib/%{cfg.buildcfg}/" )
    objdir ( "%{wks.location}/obj/%{cfg.buildcfg}/" )

    libdirs ( "%{wks.location}/lib/%{cfg.buildcfg}/" )

    filter { "system:windows", "platforms:x86" }
        libdirs { "%{wks.location}/Vendor/imgui/examples/libs/glfw/lib-vc2010-32" }

    filter { "system:windows", "platforms:x86_64" }
        libdirs { "%{wks.location}/Vendor/imgui/examples/libs/glfw/lib-vc2010-64" }


    filter { "configurations:Debug" }
        defines { "DEBUG" }
        runtime "Debug"

    filter { "configurations:Release" }
        runtime "Release"

    filter { "system:windows", "configurations:Debug" }
        ignoredefaultlibraries { "msvcrt" }

    filter { "system:windows" }
        links {  "legacy_stdio_definitions", "opengl32", "glfw3", "imgui", "implot" }
