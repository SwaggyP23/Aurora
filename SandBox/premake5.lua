project "SandBox"
    kind "WindowedApp" -- SandBox is a windowed app always since it is a testing runtime testing environment
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
        "%{IncludeDir.ImGui}", -- This is to be removed later when SandBox is cleaned
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
            "%{wks.location}/Aurora/dependencies/assimp/AssimpBin/Release/assimp-vc141-mt.lib"
        }

        postbuildcommands
        {
            ("{COPY} %{wks.location}/Aurora/dependencies/assimp/AssimpBin/Release/assimp-vc141-mt.dll %{cfg.targetdir}")
        }

    filter "configurations:Debug"
        defines "AURORA_DEBUG"
        runtime "Debug"
        symbols "on"

        links
        {
            "%{wks.location}/Aurora/dependencies/assimp/AssimpBin/Debug/assimp-vc141-mt.lib"
        }

        postbuildcommands
        {
            ("{COPY} %{wks.location}/Aurora/dependencies/assimp/AssimpBin/Debug/assimp-vc141-mt.dll %{cfg.targetdir}")
        }

    filter "configurations:Release"
        defines "AURORA_RELEASE"
        runtime "Release"
        optimize "Speed"
        inlining "Auto"

        links
        {
            "%{wks.location}/Aurora/dependencies/assimp/AssimpBin/Release/assimp-vc141-mt.lib"
        }

        postbuildcommands
        {
            ("{COPY} %{wks.location}/Aurora/dependencies/assimp/AssimpBin/Release/assimp-vc141-mt.dll %{cfg.targetdir}")
        }

    filter "configurations:Dist"
        defines "AURORA_DIST"
        runtime "Release"
        optimize "Speed"
        inlining "Auto"

        links
        {
            "%{wks.location}/Aurora/dependencies/assimp/AssimpBin/Release/assimp-vc141-mt.lib"
        }

        postbuildcommands
        {
            ("{COPY} %{wks.location}/Aurora/dependencies/assimp/AssimpBin/Release/assimp-vc141-mt.dll %{cfg.targetdir}")
        }