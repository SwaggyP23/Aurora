#include "Aurorapch.h"
#include "AssetSerializers.h"

#include "AssetManager.h"
#include "Graphics/Texture.h"
#include "Renderer/Renderer.h"

namespace Aurora {

	bool EnvironmentMapsSerializer::TryLoadData(const AssetMetaData& metadata, Ref<Asset>& asset) const
	{
		auto [radianceMap, irradianceMap] = Renderer::CreateEnvironmentMap(AssetManager::GetFileSystemPathString(metadata));

		if (!radianceMap || !irradianceMap)
			return false;

		asset = Environment::Create(radianceMap, irradianceMap);
		asset->Handle = metadata.Handle;

		return true;
	}

	bool TextureSerializer::TryLoadData(const AssetMetaData& metadata, Ref<Asset>& asset) const
	{
		asset = Texture2D::Create(AssetManager::GetFileSystemPathString(metadata));
		asset->Handle = metadata.Handle;

		bool result = asset.As<Texture2D>()->IsLoaded();

		if (!result)
			asset->SetFlag(AssetFlag::Invalid, true);

		return result;
	}

	bool FontSerializer::TryLoadData(const AssetMetaData& metadata, Ref<Asset>& asset) const
	{
		asset = Font::Create(AssetManager::GetFileSystemPathString(metadata));
		asset->Handle = metadata.Handle;

		return true;
	}

	bool MeshAssetSerializer::TryLoadData(const AssetMetaData& metadata, Ref<Asset>& asset) const
	{
		asset = MeshSource::Create(AssetManager::GetFileSystemPathString(metadata));
		if (!asset)
		{
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		asset->Handle = metadata.Handle;

		return true;
	}

	void MaterialAssetSerializer::Serialize(const AssetMetaData& metadata, const Ref<Asset>& asset) const
	{
		AR_CORE_ASSERT(false, "Not Implemented!");
	}

	bool MaterialAssetSerializer::TryLoadData(const AssetMetaData& metadata, Ref<Asset>& asset) const
	{
		AR_CORE_ASSERT(false, "Not Implemented!");
		return false;
	}

	void StaticMeshSerializer::Serialize(const AssetMetaData& metadata, const Ref<Asset>& asset) const
	{
		AR_CORE_ASSERT(false, "Not Implemented!");
	}

	bool StaticMeshSerializer::TryLoadData(const AssetMetaData& metadata, Ref<Asset>& asset) const
	{
		AR_CORE_ASSERT(false, "Not Implemented!");
		return false;
	}

}