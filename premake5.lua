include "Dependencies.lua"

workspace "Aurora"

    architecture "x86_64"
    startproject "Luna"

        configurations 
        {
            "Profile",
            "Debug",
            "Release"
        }

        flags
        {
            "MultiProcessorCompile"
        }

outputdir = "%{cfg.architecture}-%{cfg.buildcfg}"

group "Dependencies"
    include "Aurora/dependencies/GLFW"
    include "Aurora/dependencies/Glad"
    include "Aurora/dependencies/ImGui/imgui"
    include "Aurora/dependencies/optick"
group ""

group "Core"
    include "Aurora"
    include "Luna"
group ""

group "Runtime"
    include "SandBox"
group "Runtime"