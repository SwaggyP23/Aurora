
VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}

IncludeDir["GLFW"]                = "%{wks.location}/Aurora/dependencies/glfw/include"
IncludeDir["Glad"]                = "%{wks.location}/Aurora/dependencies/Glad/include"
IncludeDir["ImGui"]               = "%{wks.location}/Aurora/dependencies/ImGui"
IncludeDir["stb"]                 = "%{wks.location}/Aurora/dependencies/stb"
IncludeDir["glm"]                 = "%{wks.location}/Aurora/dependencies/glm"
IncludeDir["Optick"]              = "%{wks.location}/Aurora/dependencies/optick"
IncludeDir["VulkanSDK"]           = "%{VULKAN_SDK}/Include"
IncludeDir["Entt"]                = "%{wks.location}/Aurora/dependencies/entt"
IncludeDir["Yaml"]                = "%{wks.location}/Aurora/dependencies/yaml-cpp/include"
IncludeDir["choc"]                = "%{wks.location}/Aurora/dependencies/Choc"
IncludeDir["assimp"]              = "%{wks.location}/Aurora/dependencies/assimp/include"

LibraryDir = {}

LibraryDir["VulkanSDK"]           = "%{VULKAN_SDK}/Lib"

Library = {}

Library["AssimpDebug"]            = "%{wks.location}/Aurora/dependencies/assimp/AssimpBin/Debug/assimp-vc143-mtd.lib"
Library["AssimpRelease"]          = "%{wks.location}/Aurora/dependencies/assimp/AssimpBin/Release/assimp-vc143-mt.lib"
Library["Vulkan"]                 = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"]            = "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"
Library["dxc"]                    = "%{LibraryDir.VulkanSDK}/dxcompiler.lib"
								  
Library["ShadercDebug"]           = "%{LibraryDir.VulkanSDK}/shaderc_sharedd.lib"
Library["ShadercUtilsDebug"]      = "%{LibraryDir.VulkanSDK}/shaderc_utild.lib"
Library["SPIRV_CrossDebug"]       = "%{LibraryDir.VulkanSDK}/spirv-cross-cored.lib"
Library["SPIRV_CrossGLSLDebug"]   = "%{LibraryDir.VulkanSDK}/spirv-cross-glsld.lib"
Library["SPIRV_ToolsDebug"]       = "%{LibraryDir.VulkanSDK}/SPIRV-Toolsd.lib"
								  
Library["ShadercRelease"]         = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["ShadercUtilsRelease"]    = "%{LibraryDir.VulkanSDK}/shaderc_util.lib"
Library["SPIRV_CrossRelease"]     = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_CrossGLSLRelease"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"

Binaries = {}

Binaries["AssimpDebug"]           = "%{wks.location}/Aurora/dependencies/assimp/AssimpBin/Debug/assimp-vc143-mtd.dll"
Binaries["AssimpRelease"]         = "%{wks.location}/Aurora/dependencies/assimp/AssimpBin/Release/assimp-vc143-mt.dll"