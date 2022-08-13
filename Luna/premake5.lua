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
            "%{wks.location}/Aurora/dependencies/assimp/AssimpBin/Release/assimp-vc143-mt.lib"
        }

        postbuildcommands
        {
            ("{COPY} %{wks.location}/Aurora/dependencies/assimp/AssimpBin/Release/assimp-vc143-mt.dll %{cfg.targetdir}")
        }

    filter "configurations:Debug"
        defines "AURORA_DEBUG"
        runtime "Debug"
        symbols "on"

        links
        {
            "%{wks.location}/Aurora/dependencies/assimp/AssimpBin/Debug/assimp-vc143-mtd.lib"
        }

        postbuildcommands
        {
            ("{COPY} %{wks.location}/Aurora/dependencies/assimp/AssimpBin/Debug/assimp-vc143-mtd.dll %{cfg.targetdir}")
        }

    filter "configurations:Release"
        defines "AURORA_RELEASE"
        runtime "Release"
        optimize "Speed"
        inlining "Auto"

        links
        {
            "%{wks.location}/Aurora/dependencies/assimp/AssimpBin/Release/assimp-vc143-mt.lib"
        }

        postbuildcommands
        {
            ("{COPY} %{wks.location}/Aurora/dependencies/assimp/AssimpBin/Release/assimp-vc143-mt.dll %{cfg.targetdir}")
        }

    filter "configurations:Dist"
        defines "AURORA_DIST"
        runtime "Release"
        optimize "Speed"
        inlining "Auto"

        links
        {
            "%{wks.location}/Aurora/dependencies/assimp/AssimpBin/Release/assimp-vc143-mt.lib"
        }

        postbuildcommands
        {
            ("{COPY} %{wks.location}/Aurora/dependencies/assimp/AssimpBin/Release/assimp-vc143-mt.dll %{cfg.targetdir}")
        }

    filter { "system:windows", "configurations:Dist" }
        kind "WindowedApp"