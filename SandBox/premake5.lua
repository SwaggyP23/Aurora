project "SandBox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin/Intermedieates/" .. outputdir .. "/%{prj.name}")

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
        "%{IncludeDir.Glad}", -- These two should not be here but they give some dogshit include error
        "%{IncludeDir.GLFW}", -- These two should not be here but they give some dogshit include error
        "%{IncludeDir.ImGui}",-- This one also
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