workspace "OpenGLSeries"

    architecture "x64"

    	configurations 
    	{
    		"Debug",
    		"Release"
    	}

outputdir = "%{cfg.architecture}-%{cfg.buildcfg}"

IncludeDir = {}
IncludeDir["GLFW"] = "Application/deps/glfw/include"

include "Application/deps/glfw"

project "Application"
        location "OpenGLSeries"
        kind "ConsoleApp"
        language "C++"

        files
	    {
		    "%{prj.name}/src/**.h",
		    "%{prj.name}/src/**.cpp"
	    }

        includedirs
	    {
		    "%{prj.name}/deps/spdlog",
		    "%{prj.name}/src",
		    "%{IncludeDir.GLFW}"
	    }

	    links
	    {
	    	"GLFW",
	    	"opengl32.lib"
	    }