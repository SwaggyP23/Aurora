project "Optick"
	kind "StaticLib"
	language "C++"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin/Intermediates/" .. outputdir .. "/%{prj.name}")

	files
	{
		"optick/**.h",
		"optick/**.cpp"
	}

    removefiles "optick/optick_gpu.vulkan.cpp"

    links
    {
    }

	filter "system:windows"
        systemversion "latest"

    filter "configurations:Profile"
        runtime "Release"
        optimize "on"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "Speed"
        inlining "Auto"

    filter "configurations:Dist"
        runtime "Release"
        optimize "Speed"
        inlining "Auto"