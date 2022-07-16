project "Optick"
	kind "StaticLib"
	language "C++"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin/Intermediates/" .. outputdir .. "/%{prj.name}")

	files
	{
		"optick/**.h",
		"optick/**.cpp"
	}

    links
    {
    }

	filter "system:windows"
        systemversion "latest"

    filter "configurations:Profile"
        runtime "Debug"
        symbols "on"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "Speed"
        inlining "Auto"