-- dofile "/vendor/googletest-main/premake5.lua"
dofile "../lib/premake5.lua"


project "TestingGtestPremake"
kind "ConsoleApp"
language "C++"
cppdialect "C++20"
files {
    "*.cpp",
    "*.h",
    "*.hpp"
    }

    libdirs{
        "../vendor/googletest-main/bin/Debug/"

    }
    links { 
    "MyNumericLibrary",
    "gtest"
        }
    includedirs { 
        "../lib/headers/",
        "../vendor/googletest-main/googletest/include/" ,
        "../vendor/googletest-main/googletest/include/gtest" 
        }
    targetdir ("bin/%{cfg.buildcfg}") 

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

