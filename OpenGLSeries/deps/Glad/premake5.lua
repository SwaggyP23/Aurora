project "Glad"
    kind "StaticLib"
    language "C"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin/Intermediates/" .. outputdir .. "/%{prj.name}")

    files
    {
        "include/glad/glad.h",
        "include/KHR/khrplatform.h",
        "src/glad.c"
    }

    includedirs
    {
        "include"
    }
    
    filter "system:windows"
        systemversion "latest"

    filter "configurations:Profile"
        runtime "Debug"
        symbols "on"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"

    filter { "system:windows", "configurations:Release" }
        buildoptions "/O2"