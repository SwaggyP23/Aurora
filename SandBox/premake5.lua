project "SandBox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin/Intermedieates/" .. outputdir .. "/%{prj.name}")

    files
    {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs
    {
        "%{wks.location}/OpenGLSeries/deps/spdlog/include",
        "%{wks.location}/OpenGLSeries/src",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.glm}"
    }

    links
    {
        "OpenGLSeries"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
        }

    filter "configurations:Profile"
        defines
        {
            "_DEBUG",
            "_CORE_PROFILE_"
        }

        runtime "Debug"
        symbols "on"

        links
        {
        }

    filter "configurations:Debug"
        defines "_DEBUG"
        runtime "Debug"
        symbols "on"

        links
        {
        }

    filter "configurations:Release"
        defines "_RELEASE"
        runtime "Release"
        optimize "on"

        links
        {
        }

    filter { "system:windows", "configurations:Release" }
        buildoptions "/O2"