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

    postbuildcommands
    {
        ("{COPY} %{wks.location}/Aurora/dependencies/assimp/bin/" .. outputdir .. "/Assimp/Assimp.dll %{cfg.targetdir}")
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