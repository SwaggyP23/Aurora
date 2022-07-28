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
        "dependencies/stb_image/**.h",
        "dependencies/stb_image/**.cpp",
        "dependencies/glm/glm/**.hpp",
        "dependencies/glm/glm/**.inl",

        "dependencies/ImGuizmo/ImGuizmo/ImGuizmo.h",
        "dependencies/ImGuizmo/ImGuizmo/ImGuizmo.cpp"
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
        "%{IncludeDir.ImGuizmo}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.Entt}",
        "%{IncludeDir.Yaml}",
        "%{IncludeDir.Optick}",
        "%{IncludeDir.choc}"
    }

    links
    {
        "GLFW",
        "Glad",
        "ImGui",
        "Optick",
        "yaml-cpp",
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

    filter "configurations:Dist"
        defines "AURORA_DIST"
        runtime "Release"
        optimize "Speed"
        inlining "Auto"

        links
        {
        }

    filter "files:dependencies/stb_image/**.cpp"
        flags { "NoPCH" }

    filter "files:dependencies/ImGuizmo/**cpp"
        flags { "NoPCH" }