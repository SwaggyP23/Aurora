project "OpenGLSeries"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "Off"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin/Intermedieates/" .. outputdir .. "/%{prj.name}")

	pchheader "OGLpch.h"
	pchsource "src/OGLpch.cpp"

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
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.stb_image}"
	}
	links
	{
        "GLFW",
        "Glad",
        "ImGui",
        "opengl32.lib"
	}

	filter "system:windows"
		systemversion "latest"

		defines
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