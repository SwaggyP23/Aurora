project "Aurora"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin/Intermedieates/" .. outputdir .. "/%{prj.name}")

    pchheader "Aurorapch.h"
    pchsource "src/Aurorapch.cpp"

    files
    {
        "src/**.h",
        "src/**.cpp",
        "deps/stb_image/**.h",
        "deps/stb_image/**.cpp",
        "deps/glm/glm/**.hpp",
        "deps/glm/glm/**.inl"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "GLFW_INCLUDE_NONE"
    }

    includedirs
    {
        "src",
        "deps/spdlog/include",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.Optick}",
        "%{IncludeDir.Entt}"
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
        }

    filter "configurations:Profile"
        defines
        {
            "AURORA_DEBUG",
            "AURORA_CORE_PROFILE"
        }

        runtime "Debug"
        symbols "on"

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

    filter "files:deps/stb_image/**.cpp"
        flags { "NoPCH" }