#pragma once

#include "AssetMetaData.h"
#include "AssetRegistry.h"
#include "AssetImporter.h"

namespace Aurora {

	// TODO: Add on filesystem changed events for deleting assets and renaming assets

	// TODO: Move into projects
	static const char* s_AssetPath = "SandboxProject/Assets";
	static const char* s_AssetRegistryPath = "SandboxProject/Assets/AssetRegistry.azr";

	class AssetManager
	{
	public:
		static void Init();
		static void Shutdown();

		static const AssetMetaData& GetMetaData(AssetHandle handle);
		static const AssetMetaData& GetMetaData(const std::filesystem::path& filepath);

		static std::filesystem::path GetFileSystemPath(const AssetMetaData& metadata) { return std::filesystem::path(s_AssetPath) / metadata.FilePath; }
		static std::string GetFileSystemPathString(const AssetMetaData& metadata) { return GetFileSystemPath(metadata).string(); }
		static std::filesystem::path GetRelativePath(const std::filesystem::path& filePath);

		static AssetHandle GetAssetHandleFromFilePath(const std::filesystem::path& filePath);
		static bool IsAssetHandleValid(AssetHandle assetHandle) { return IsMemoryAsset(assetHandle) || GetMetaData(assetHandle).IsValid(); }

		static AssetType GetAssetTypeFromExtension(const std::string& extension);
		static AssetType GetAssetTypeFromPath(const std::filesystem::path& filePath);

		static AssetHandle ImportAsset(const std::filesystem::path& filePath);
		static bool ReloadData(AssetHandle assetHandle);

		template<typename T, typename... Args>
		static Ref<T> CreateNewAsset(const std::string& fileName, const std::string& directoryPath, Args&&... args)
		{
			static_assert(std::is_base_of<Asset, T>::value, "CreateNewAsset only works for types that inherit from Asset");

			AssetMetaData metaData = {};
			metaData.Handle = AssetHandle();
			if (directoryPath.empty() || directoryPath == ".")
				metaData.FilePath = fileName;
			else
				metaData.FilePath = GetRelativePath(directoryPath + "/" + fileName);
			metaData.IsDataLoaded = false;
			metaData.Type = T::GetStaticType();

			if (FileExists(metaData))
			{
				bool foundAvailableFileName = false;
				int current = 1;

				while (!foundAvailableFileName)
				{
					std::string nextFilePath = directoryPath + "/" + metaData.FilePath.stem().string();

					if (current < 10)
						nextFilePath += "(0" + std::to_string(current) + ")";
					else
						nextFilePath += "(" + std::to_string(current) + ")";
					nextFilePath += metaData.FilePath.extension().string();

					if (!std::filesystem::exists(s_AssetPath / GetRelativePath(nextFilePath)))
					{
						foundAvailableFileName = true;
						metaData.FilePath = GetRelativePath(nextFilePath);
						break;
					}

					current++;
				}
			}

			s_AssetRegistry[metaData.Handle] = metaData;

			WriteRegistryToFile();

			Ref<T> asset = T::Create(std::forward<Args>(args)...); // This assumes every class that inherits Asset should have a static Create functions which is the convention!
			asset->Handle = metaData.Handle;
			s_LoadedAssets[asset->Handle] = asset;
			AssetImporter::Serialize(metaData, asset);

			return asset;
		}

		template<typename T>
		static Ref<T> GetAsset(AssetHandle assetHandle)
		{
			AR_PROFILE_FUNCTION();

			if (IsMemoryAsset(assetHandle))
				return s_MemoryAssets[assetHandle].As<T>();

			AssetMetaData& metaData = GetMetaDataInternal(assetHandle);
			if (!metaData.IsValid())
				return nullptr;

			Ref<Asset> asset = nullptr;
			if (!metaData.IsDataLoaded)
			{
				metaData.IsDataLoaded = AssetImporter::TryLoadData(metaData, asset);
				if (!metaData.IsDataLoaded)
					return nullptr;

				s_LoadedAssets[assetHandle] = asset;
			}
			else
			{
				asset = s_LoadedAssets[assetHandle];
			}

			return asset.As<T>();
		}

		template<typename T>
		static Ref<T> GetAsset(const std::filesystem::path& filePath)
		{
			return GetAsset<T>(GetAssetHandleFromFilePath(filePath));
		}

		static bool FileExists(AssetMetaData& metaData)
		{
			return std::filesystem::exists(s_AssetPath / metaData.FilePath);
		}

		static const std::unordered_map<AssetHandle, Ref<Asset>>& GetLoadedAssets() { return s_LoadedAssets; }
		static const AssetRegistry& GetAssetRegistry() { return s_AssetRegistry; }
		static const std::unordered_map<AssetHandle, Ref<Asset>>& GetMemoryOnlyAssets() { return s_MemoryAssets; }

		template<typename T, typename... Args>
		static AssetHandle CreateMemoryOnlyAsset(Args&&... args)
		{
			static_assert(std::is_base_of<Asset, T>::value, "CreateMemoryOnlyAsset only works with types that inherit from Asset");

			Ref<T> asset = T::Create(std::forward<Args>(args)...);
			asset->Handle = AssetHandle();

			s_MemoryAssets[asset->Handle] = asset;

			return asset->Handle;
		}

		template<typename T>
		static AssetHandle AddMemoryOnlyAsset(Ref<T> asset)
		{
			static_assert(std::is_base_of<Asset, T>::value, "AddMemoryOnlyAsset only works for types that inherit from Asset");

			asset->Handle = AssetHandle();
			s_MemoryAssets[asset->Handle] = asset;

			return asset->Handle;
		}

		template<typename T, typename... Args>
		static AssetHandle CreateNamedMemoryOnlyAsset(const std::string& name, Args&&... args)
		{
			static_assert(std::is_base_of<Asset, T>::value, "CreateNamedMemoryOnlyAsset only works for types that inherit from Asset");

			Ref<T> asset = T::Create(std::forward<Args>(args)...);
			asset->Handle = AssetHandle();

			AssetMetaData metaData = {};
			metaData.Handle = asset->Handle;
			metaData.FilePath = name;
			metaData.IsDataLoaded = true;
			metaData.Type = T::GetStaticType();
			metaData.IsMemoryAsset = true;

			s_AssetRegistry[metaData.Handle] = metaData;
			s_MemoryAssets[metaData.Handle] = asset;

			return asset->Handle;
		}

		static bool IsMemoryAsset(AssetHandle handle)
		{
			return s_MemoryAssets.find(handle) != s_MemoryAssets.end();
		}

	private:
		static void LoadAssetRegistry();
		static void WriteRegistryToFile();
		static void ProcessDirectory(const std::filesystem::path& directoryPath);
		static void ReloadAssets();

		static AssetMetaData& GetMetaDataInternal(AssetHandle handle);

		static void OnAssetRenamed(AssetHandle assetHandle, const std::filesystem::path& newFilePath);
		static void OnAssetDeleted(AssetHandle assetHandle);

	private:
		static std::unordered_map<AssetHandle, Ref<Asset>> s_LoadedAssets;
		static std::unordered_map<AssetHandle, Ref<Asset>> s_MemoryAssets;

		static AssetRegistry s_AssetRegistry;

		// TODO: Should be contentBrowserPanel once split
		friend class EditorLayer;
			
	};

}