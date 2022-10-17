#include "Aurorapch.h"
#include "AssetImporter.h"

#include "AssetManager.h"

namespace Aurora {

	void AssetImporter::Init()
	{
		s_Serializers[AssetType::Texture] = CreateScope<TextureSerializer>();
		s_Serializers[AssetType::EnvironmentMap] = CreateScope<EnvironmentMapsSerializer>();
		s_Serializers[AssetType::StaticMesh] = CreateScope<StaticMeshSerializer>();
		s_Serializers[AssetType::MeshSource] = CreateScope<MeshAssetSerializer>();
		s_Serializers[AssetType::Material] = CreateScope<MaterialAssetSerializer>();
		//s_Serializers[AssetType::Scene] = CreateScope<SceneAssetSerializer>(); // TODO: Not Implemented!
		s_Serializers[AssetType::Font] = CreateScope<FontSerializer>();
	}

	void AssetImporter::Shutdown()
	{
	}

	void AssetImporter::Serialize(const AssetMetaData& metaData, const Ref<Asset>& asset)
	{
		if (s_Serializers.find(metaData.Type) == s_Serializers.end())
		{
			AR_CORE_WARN_TAG("AssetImporter", "There's currently no importer for assets of type {0}", metaData.FilePath.stem().string());
			return;
		}

		s_Serializers[asset->GetAssetType()]->Serialize(metaData, asset);
	}

	void AssetImporter::Serialize(const Ref<Asset>& asset)
	{
		const AssetMetaData& metadata = AssetManager::GetMetaData(asset->Handle);
		Serialize(metadata, asset);
	}

	bool AssetImporter::TryLoadData(const AssetMetaData& metaData, Ref<Asset>& asset)
	{
		if (s_Serializers.find(metaData.Type) == s_Serializers.end())
		{
			AR_CORE_WARN_TAG("AssetImporter", "There's currently no importer for assets of type {0}", metaData.FilePath.stem().string());
			return false;
		}

		return s_Serializers[metaData.Type]->TryLoadData(metaData, asset);
	}

	std::unordered_map<AssetType, Scope<AssetSerializer>> AssetImporter::s_Serializers;

}