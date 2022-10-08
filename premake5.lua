include "Dependencies.lua"

workspace "Aurora"

    architecture "x86_64"
    startproject "Luna"

        configurations 
        {
            "Profile",
            "Debug",
            "Release",
            "Dist"
        }

        flags
        {
            "MultiProcessorCompile"
        }

outputdir = "%{cfg.architecture}-%{cfg.buildcfg}"

group "Dependencies"
    include "Aurora/dependencies/glfw"
    include "Aurora/dependencies/Glad"
    include "Aurora/dependencies/ImGui/imgui"
    include "Aurora/dependencies/optick"
    include "Tools/BuildSystem"
group ""

group "Core"
    include "Aurora"
    include "Luna"
group ""

group "Runtime"
    include "Aurora-Runtime"
group "Runtime"