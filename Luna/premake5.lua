project "Luna"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin/Intermediates/" .. outputdir .. "/%{prj.name}")

    files
    {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs
    {
        "%{wks.location}/Aurora/src",
        "%{wks.location}/Aurora/dependencies/spdlog/include",
        "%{wks.location}/Aurora/dependencies",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.Entt}",
        "%{IncludeDir.Optick}"
    }

    links
    {
        "Aurora"
    }

    defines
    {
        "GLM_FORCE_DEPTH_ZERO_TO_ONE"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "AURORA_PLATFORM_WINDOWS"
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
            "%{Library.AssimpRelease}"
        }

        postbuildcommands
        {
            ("{COPY} %{Binaries.AssimpRelease} %{cfg.targetdir}")
        }

    filter "configurations:Debug"
        defines "AURORA_DEBUG"
        runtime "Debug"
        symbols "on"

        links
        {
            "%{Library.AssimpDebug}"
        }

        postbuildcommands
        {
            ("{COPY} %{Binaries.AssimpDebug} %{cfg.targetdir}")
        }

    filter "configurations:Release"
        defines "AURORA_RELEASE"
        runtime "Release"
        optimize "Speed"
        inlining "Auto"

        links
        {
            "%{Library.AssimpRelease}"
        }

        postbuildcommands
        {
            ("{COPY} %{Binaries.AssimpRelease} %{cfg.targetdir}")
        }

    filter "configurations:Dist"
        defines "AURORA_DIST"
        runtime "Release"
        optimize "Speed"
        inlining "Auto"

        links
        {
            "%{Library.AssimpRelease}"
        }

        postbuildcommands
        {
            ("{COPY} %{Binaries.AssimpRelease} %{cfg.targetdir}")
        }

    filter { "system:windows", "configurations:Dist" }
        kind "WindowedApp"