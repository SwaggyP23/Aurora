project "Assimp"
	kind "SharedLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin/Intermediates/" .. outputdir .. "/%{prj.name}")

    files
    {
        "src/**.c",
        "src/**.cc",
        "src/**.cpp",
        "src/**.rc",
        "src/**.h",
        "src/**.hpp",
        "src/**.inl",

        "include/assimp/**.c",
        "include/assimp/**.cpp",
        "include/assimp/**.h",
        "include/assimp/**.hpp",
        "include/assimp/**.inl"
    }

    includedirs
    {
        "src/code",
        "src/contrib",
        "%{IncludeDir.zlib}",
        "%{IncludeDir.assimp}"
    }

    links
    {
        "zlibStatic"
    }

    defines
    {
        "NDEBUG",
        "ASSIMP_BUILD_DLL_EXPORT",
        "ASSIMP_BUILD_NO_M3D_IMPORTER",
        "ASSIMP_BUILD_NO_M3D_EXPORTER",
        "ASSIMP_BUILD_NO_C4D_IMPORTER",
        "MINIZ_USE_UNALIGNED_LOADS_AND_STORES=0",
        "ASSIMP_IMPORTER_GLTF_USE_OPEN3DGC=1",
        "RAPIDJSON_HAS_STDSTRING=1",
        "RAPIDJSON_NOMEMBERITERATORCLASS",
        "_SCL_SECURE_NO_WARNINGS",
        "_CRT_SECURE_NO_WARNINGS",
        "CMAKE_INTDIR=\"Release\"",
        "OPENDDLPARSER_BUILD",
        "assimp_EXPORTS",
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