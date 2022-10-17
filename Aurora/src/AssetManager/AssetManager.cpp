#include "Aurorapch.h"
#include "AssetManager.h"

#include "Utils/UtilFunctions.h"

#include <yaml-cpp/yaml.h>

namespace Aurora {

    static AssetMetaData s_NullMetaData;

    void AssetManager::Init()
    {
        s_AssetRegistry.Clear();
        AssetImporter::Init();

        LoadAssetRegistry();
        ReloadAssets();
    }

    void AssetManager::Shutdown()
    {
        WriteRegistryToFile();

        AssetImporter::Shutdown();

        s_MemoryAssets.clear();
        s_AssetRegistry.Clear();
        s_LoadedAssets.clear();
    }

    const AssetMetaData& AssetManager::GetMetaData(AssetHandle handle)
    {
        if (s_AssetRegistry.Contains(handle))
            return s_AssetRegistry[handle];

        return s_NullMetaData;
    }

    const AssetMetaData& AssetManager::GetMetaData(const std::filesystem::path& filepath)
    {
        const std::filesystem::path relativePath = GetRelativePath(filepath);

        for (auto& [handle, metaData] : s_AssetRegistry)
        {
            if (metaData.FilePath == relativePath)
                return metaData;
        }

        return s_NullMetaData;
    }

    std::filesystem::path AssetManager::GetRelativePath(const std::filesystem::path& filePath)
    {
        std::filesystem::path relativePath = filePath.lexically_normal();

        std::string temp = filePath.string();

        if (temp.find(s_AssetPath) != std::string::npos)
        {
            relativePath = std::filesystem::relative(filePath, s_AssetPath);
            if (relativePath.empty())
            {
                relativePath = filePath.lexically_normal();
            }
        }

        return relativePath;
    }

    AssetHandle AssetManager::GetAssetHandleFromFilePath(const std::filesystem::path& filePath)
    {
        return GetMetaData(filePath).Handle;
    }

    AssetType AssetManager::GetAssetTypeFromExtension(const std::string& extension)
    {
        std::string exten = Utils::StringUtils::ToLower(extension);
        if (s_AssetExtensionsMap.find(exten) == s_AssetExtensionsMap.end())
            return AssetType::None;

        return s_AssetExtensionsMap.at(exten);
    }

    AssetType AssetManager::GetAssetTypeFromPath(const std::filesystem::path& filePath)
    {
        return GetAssetTypeFromExtension(filePath.extension().string());
    }

    AssetHandle AssetManager::ImportAsset(const std::filesystem::path& filePath)
    {
        std::filesystem::path path = GetRelativePath(filePath);

        const AssetMetaData& metaData = GetMetaData(path);
        if (metaData.IsValid())
            return metaData.Handle;

        AssetType type = GetAssetTypeFromPath(path);
        if (type == AssetType::None)
            return 0;

        AssetMetaData data = {};
        data.Handle = AssetHandle();
        data.FilePath = path;
        data.Type = type;
        s_AssetRegistry[data.Handle] = data;

        return data.Handle;
    }

    bool AssetManager::ReloadData(AssetHandle assetHandle)
    {
        AssetMetaData& data = GetMetaDataInternal(assetHandle);
        if (!data.IsValid())
        {
            AR_CORE_ERROR_TAG("AssetManeger", "Trying to reload invalid asset!");
            return false;
        }

        Ref<Asset> asset = nullptr;
        data.IsDataLoaded = AssetImporter::TryLoadData(data, asset);
        if (data.IsDataLoaded)
        {
            s_LoadedAssets[assetHandle] = asset;
        }

        return data.IsDataLoaded;
    }

    void AssetManager::LoadAssetRegistry()
    {
        AR_CORE_INFO_TAG("AssetManager", "Loading AssetRegistry");

        const std::filesystem::path& assetRegistryPath = s_AssetRegistryPath;
        if (!std::filesystem::exists(assetRegistryPath))
            return;

        std::ifstream stream(assetRegistryPath); // Closes with RAII
        AR_CORE_ASSERT(stream);
        std::stringstream sstr;

        sstr << stream.rdbuf();
        YAML::Node data = YAML::Load(sstr.str());

        YAML::Node handles = data["Assets"];
        if (!handles)
        {
            AR_CORE_ERROR_TAG("AssetManager", "AssetRegistry corrupted!");
            AR_CORE_CHECK(false);
            return;
        }

        for (auto entry : handles)
        {
            std::string filePath = entry["FilePath"].as<std::string>();

            AssetMetaData data = {};
            data.Handle = entry["Handle"].as<uint64_t>();
            data.FilePath = filePath;
            data.Type = (AssetType)Utils::AssetTypeFromString(entry["Type"].as<std::string>());

            if (data.Type == AssetType::None)
                continue;

            if (data.Type != GetAssetTypeFromPath(filePath))
            {
                AR_CORE_WARN_TAG("AssetManager", "Mismatch between stored AssetType and extension type when reading asset registry");
                data.Type = GetAssetTypeFromPath(filePath);
            }

            if (!std::filesystem::exists(GetFileSystemPath(data)))
            {
                AR_CORE_WARN_TAG("AssetManager", "Missing asset {0} detected in asset registry file, trying to locat...", data.FilePath);

                std::string mostLikelyCandidate;
                uint32_t bestScore = 0;

                for (const auto& pathEntry : std::filesystem::recursive_directory_iterator{ s_AssetPath })
                {
                    const std::filesystem::path& path = pathEntry.path();

                    if (path.filename() != data.FilePath.filename())
                        continue;

                    if (bestScore > 0)
                        AR_CORE_WARN_TAG("AssetManager", "Multiple candidates found");

                    std::vector<std::string> candidateParts = Utils::StringUtils::SplitString(path.string(), "/\\");

                    uint32_t score = 0;
                    for (const std::string& part : candidateParts)
                    {
                        if (filePath.find(part) != std::string::npos)
                            score++;
                    }

                    AR_CORE_WARN_TAG("AssetManager", "{0} has a score of {1}, best score is {2}", path.string(), score, bestScore);

                    if (bestScore > 0 && score == bestScore)
                    {
                        // TODO: ???
                    }

                    if (score <= bestScore)
                        continue;

                    bestScore = score;
                    mostLikelyCandidate = path.string();
                }

                if (mostLikelyCandidate.empty() && bestScore == 0)
                {
                    AR_CORE_ERROR_TAG("AssetManager", "Failed to loacte a potential match for {0}", data.FilePath);
                    continue;
                }

                std::replace(mostLikelyCandidate.begin(), mostLikelyCandidate.end(), '\\', '/');
                data.FilePath = std::filesystem::relative(mostLikelyCandidate, s_AssetPath);
                AR_CORE_WARN_TAG("AssetManager", "Found most likely match {0}", data.FilePath);
            }

            if (data.Handle == 0)
            {
                AR_CORE_WARN_TAG("AssetManager", "AssetHandle for {0} is 0, which is wrong!", data.FilePath);
                continue;
            }

            s_AssetRegistry[data.Handle] = data;
        }

        AR_CORE_INFO_TAG("AssetManager", "Loaded {0} asset entries", s_AssetRegistry.Count());
    }

    void AssetManager::WriteRegistryToFile()
    {
        struct AssetRegistryEntry
        {
            std::string FilePath;
            AssetType Type;
        };

        std::map<UUID, AssetRegistryEntry> sortedMap;

        for (auto& [filePath, data] : s_AssetRegistry)
        {
            if (!std::filesystem::exists(GetFileSystemPath(data)))
                continue;

            if (data.IsMemoryAsset)
                continue;

            std::string pathToSerialize = data.FilePath.string();
            std::replace(pathToSerialize.begin(), pathToSerialize.end(), '\\', '/');
            sortedMap[data.Handle] = { pathToSerialize, data.Type };
        }

        AR_CORE_INFO_TAG("AssetManager", "Serializing asset registry with {0} asset entries", sortedMap.size());

        YAML::Emitter out;
        out << YAML::BeginMap;

        out << YAML::Key << "Assets" << YAML::BeginSeq;
        for (auto& [handle, entry] : sortedMap)
        {
            out << YAML::BeginMap;

            out << YAML::Key << "Handle" << YAML::Value << handle;
            out << YAML::Key << "FilePath" << YAML::Value << entry.FilePath;
            out << YAML::Key << "Type" << YAML::Value << Utils::AssetTypeToString(entry.Type);

            out << YAML::EndMap;
        }
        out << YAML::EndSeq;

        out << YAML::EndMap;

        const std::string& assetRegistryPath = s_AssetRegistryPath;
        std::ofstream ofStream(assetRegistryPath);
        ofStream << out.c_str();
    }

    void AssetManager::ProcessDirectory(const std::filesystem::path& directoryPath)
    {
        for (const auto& entry : std::filesystem::directory_iterator{ directoryPath })
        {
            if (entry.is_directory())
                ProcessDirectory(entry.path());
            else
                ImportAsset(entry.path());
        }
    }

    void AssetManager::ReloadAssets()
    {
        ProcessDirectory(s_AssetPath);
        WriteRegistryToFile();
    }

    AssetMetaData& AssetManager::GetMetaDataInternal(AssetHandle handle)
    {
        if (s_AssetRegistry.Contains(handle))
            return s_AssetRegistry[handle];

        return s_NullMetaData; // make sure to check return value before you go changing it!
    }

    void AssetManager::OnAssetRenamed(AssetHandle assetHandle, const std::filesystem::path& newFilePath)
    {
        AssetMetaData metaData = GetMetaData(assetHandle);
        if (!metaData.IsValid())
            return;

        metaData.FilePath = GetRelativePath(newFilePath);
        WriteRegistryToFile();
    }

    void AssetManager::OnAssetDeleted(AssetHandle assetHandle)
    {
        AssetMetaData metaData = GetMetaData(assetHandle);
        if (!metaData.IsValid())
            return;

        s_AssetRegistry.Remove(assetHandle);
        s_LoadedAssets.erase(assetHandle);
        WriteRegistryToFile();
    }

    std::unordered_map<AssetHandle, Ref<Asset>> AssetManager::s_LoadedAssets;
    std::unordered_map<AssetHandle, Ref<Asset>> AssetManager::s_MemoryAssets;

    AssetRegistry AssetManager::s_AssetRegistry;

}