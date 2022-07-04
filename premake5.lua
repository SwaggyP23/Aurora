IncludeDir = {}
IncludeDir["GLFW"]      = "%{wks.location}/OpenGLSeries/deps/GLFW/include"
IncludeDir["Glad"]      = "%{wks.location}/OpenGLSeries/deps/Glad/include"
IncludeDir["ImGui"]     = "%{wks.location}/OpenGLSeries/deps/ImGui"
IncludeDir["stb_image"] = "%{wks.location}/OpenGLSeries/deps/stb_image"
IncludeDir["glm"]       = "%{wks.location}/OpenGLSeries/deps/glm"

workspace "OpenGLSeries"

    architecture "x64"

        configurations 
        {
            "Debug",
            "Release"
        }

outputdir = "%{cfg.architecture}-%{cfg.buildcfg}"

group "Dependencies"
    include "OpenGLSeries/deps/GLFW"
    include "OpenGLSeries/deps/Glad"
    include "OpenGLSeries/deps/ImGui/imgui"
group ""

include "OpenGLSeries"
include "SandBox"