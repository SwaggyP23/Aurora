IncludeDir = {}
IncludeDir["GLFW"]      = "%{wks.location}/Aurora/deps/GLFW/include"
IncludeDir["Glad"]      = "%{wks.location}/Aurora/deps/Glad/include"
IncludeDir["ImGui"]     = "%{wks.location}/Aurora/deps/ImGui"
IncludeDir["stb_image"] = "%{wks.location}/Aurora/deps/stb_image"
IncludeDir["glm"]       = "%{wks.location}/Aurora/deps/glm"

workspace "Aurora"

    architecture "x64"

        configurations 
        {
            "Profile",
            "Debug",
            "Release"
        }

outputdir = "%{cfg.architecture}-%{cfg.buildcfg}"

group "Dependencies"
    include "Aurora/deps/GLFW"
    include "Aurora/deps/Glad"
    include "Aurora/deps/ImGui/imgui"
group ""

include "Aurora"
include "SandBox"