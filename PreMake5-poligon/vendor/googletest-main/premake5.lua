project "gtest"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    
    files { 
        "googletest/src/*.cc",
        "googletest/include/gtest/*.h"
    }
    
    includedirs { 
        "googletest/include/",
        "googletest/include/gtest/",
        "googletest/src/",
        "googletest/"
    }
    targetdir ("bin/%{cfg.buildcfg}")

    -- Configuration-specific settings
    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"