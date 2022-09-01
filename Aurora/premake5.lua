project "Aurora"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin/Intermediates/" .. outputdir .. "/%{prj.name}")

    pchheader "Aurorapch.h"
    pchsource "src/Aurorapch.cpp"

    files
    {
        "src/**.h",
        "src/**.cpp",

        "dependencies/stb/**.h",
        "dependencies/stb/**.cpp",

        "dependencies/yaml-cpp/include/**.h",
        "dependencies/yaml-cpp/src/**.h",
        "dependencies/yaml-cpp/src/**.cpp"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "GLFW_INCLUDE_NONE",
        "GLM_FORCE_DEPTH_ZERO_TO_ONE"
    }

    includedirs
    {
        "src",
        "dependencies/spdlog/include",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.VulkanSDK}",
        "%{IncludeDir.stb}",
        "%{IncludeDir.Entt}",
        "%{IncludeDir.Yaml}",
        "%{IncludeDir.choc}",
        "%{IncludeDir.assimp}",

        "%{IncludeDir.Optick}"
    }

    links
    {
        "GLFW",
        "Glad",
        "ImGui",
        "Optick",
        "opengl32.lib",
        "%{Library.Vulkan}",
        "%{Library.VulkanUtils}"
    }

    postbuildmessage "Core: Done building Aurora!"

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "AURORA_PLATFORM_WINDOWS"
        }

    filter "configurations:Profile"
        defines
        {
            "AURORA_RELEASE",
            "AURORA_CORE_PROFILE"
        }

        runtime "Release"
        optimize "on"

        links
        {
            "%{Library.dxc}",
            "%{Library.ShadercRelease}",
            "%{Library.ShadercUtilsRelease}",
            "%{Library.SPIRV_CrossRelease}",
            "%{Library.SPIRV_CrossGLSLRelease}",
            "%{Library.SPIRV_ToolsRelease}"
        }

    filter "configurations:Debug"
        defines "AURORA_DEBUG"
        runtime "Debug"
        symbols "on"

        links
        {
            "%{Library.dxc}",
            "%{Library.ShadercDebug}",
            "%{Library.ShadercUtilsDebug}",
            "%{Library.SPIRV_CrossDebug}",
            "%{Library.SPIRV_CrossGLSLDebug}",
            "%{Library.SPIRV_ToolsDebug}"
        }

    filter "configurations:Release"
        defines "AURORA_RELEASE"
        runtime "Release"
        optimize "Speed"
        inlining "Auto"

        links
        {
            "%{Library.dxc}",
            "%{Library.ShadercRelease}",
            "%{Library.ShadercUtilsRelease}",
            "%{Library.SPIRV_CrossRelease}",
            "%{Library.SPIRV_CrossGLSLRelease}",
            "%{Library.SPIRV_ToolsRelease}"
        }

    filter "configurations:Dist"
        defines "AURORA_DIST"
        runtime "Release"
        optimize "Speed"
        inlining "Auto"

        links
        {
            "%{Library.dxc}",
            "%{Library.ShadercRelease}",
            "%{Library.ShadercUtilsRelease}",
            "%{Library.SPIRV_CrossRelease}",
            "%{Library.SPIRV_CrossGLSLRelease}",
            "%{Library.SPIRV_ToolsRelease}"
        }

    filter "files:dependencies/stb/**.cpp"
        flags { "NoPCH" }

    filter "files:dependencies/yaml-cpp/src/**.cpp"
        flags { "NoPCH" }

    filter "files:src/ImGui/imgui_demo.cpp"
        flags { "NoPCH" }