project "zlibStatic"
	kind "StaticLib"
    language "C"
    staticruntime "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin/Intermediates/" .. outputdir .. "/%{prj.name}")

    files
    {
        "zlib/**.h",
        "zlib/**.hpp",
        "zlib/**.c",
        "zlib/**.cpp"
    }

    includedirs
    {
        "zlib"
    }

    defines
    {
        "NDEBUG",
        "ASSIMP_BUILD_NO_M3D_IMPORTER",
        "ASSIMP_BUILD_NO_M3D_EXPORTER",
        "NO_FSEEKO",
        "_CRT_SECURE_NO_DEPRECATE",
        "_CRT_NONSTDC_NO_DEPRECATE",
        "CMAKE_INTDIR=\"Release\""
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

    filter "configurations:Dist"
        runtime "Release"
        optimize "Speed"
        inlining "Auto"