project "Aurora"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin/Intermediates/" .. outputdir .. "/%{prj.name}")

    pchheader "Aurorapch.h"
    pchsource "src/Aurorapch.cpp"

    files
    {
        "src/**.h",
        "src/**.cpp",

        "dependencies/stb/**.h",
        "dependencies/stb/**.cpp",

        "dependencies/yaml-cpp/include/**.h",
        "dependencies/yaml-cpp/src/**.h",
        "dependencies/yaml-cpp/src/**.cpp"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "GLFW_INCLUDE_NONE"
    }

    includedirs
    {
        "src",
        "dependencies/spdlog/include",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.stb}",
        "%{IncludeDir.Entt}",
        "%{IncludeDir.Yaml}",
        "%{IncludeDir.choc}",
        "%{IncludeDir.assimp}",

        "%{IncludeDir.Optick}"
    }

    links
    {
        "GLFW",
        "Glad",
        "ImGui",
        "Optick",
        "opengl32.lib"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "AR_PLATFORM_WINDOWS"
        }

    filter "configurations:Profile"
        defines
        {
            "AURORA_RELEASE",
            "AURORA_CORE_PROFILE"
        }

        runtime "Release"
        optimize "on"

        links
        {
        }

    filter "configurations:Debug"
        defines "AURORA_DEBUG"
        runtime "Debug"
        symbols "on"

        links
        {
        }

    filter "configurations:Release"
        defines "AURORA_RELEASE"
        runtime "Release"
        optimize "Speed"
        inlining "Auto"

        links
        {
        }

    filter "configurations:Dist"
        defines "AURORA_DIST"
        runtime "Release"
        optimize "Speed"
        inlining "Auto"

        links
        {
        }

    filter "files:dependencies/stb/**.cpp"
        flags { "NoPCH" }

    filter "files:dependencies/yaml-cpp/src/**.cpp"
        flags { "NoPCH" }