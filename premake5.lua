include "Dependencies.lua"

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

group "Core"
    include "Aurora"
    include "Luna"
group ""

group "Runtime"
    include "SandBox"
group "Runtime"