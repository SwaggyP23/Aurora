#pragma once

#include "AssetMetaData.h"

namespace Aurora {

	class AssetSerializer
	{
	public:
		virtual void Serialize(const AssetMetaData& metadata, const Ref<Asset>& asset) const = 0;
		virtual bool TryLoadData(const AssetMetaData& metadata, Ref<Asset>& asset) const = 0;
	};

	class EnvironmentMapsSerializer : public AssetSerializer
	{
		virtual void Serialize(const AssetMetaData& metadata, const Ref<Asset>& asset) const override {}
		virtual bool TryLoadData(const AssetMetaData& metadata, Ref<Asset>& asset) const override;
	};

	class TextureSerializer : public AssetSerializer
	{
	public:
		virtual void Serialize(const AssetMetaData& metadata, const Ref<Asset>& asset) const override {}
		virtual bool TryLoadData(const AssetMetaData& metadata, Ref<Asset>& asset) const override;
	};

	class FontSerializer : public AssetSerializer
	{
	public:
		virtual void Serialize(const AssetMetaData& metadata, const Ref<Asset>& asset) const override {}
		virtual bool TryLoadData(const AssetMetaData& metadata, Ref<Asset>& asset) const override;
	};

	class MeshAssetSerializer : public AssetSerializer
	{
	public:
		virtual void Serialize(const AssetMetaData& metadata, const Ref<Asset>& asset) const override {}
		virtual bool TryLoadData(const AssetMetaData& metadata, Ref<Asset>& asset) const override;
	};

	class MaterialAssetSerializer : public AssetSerializer
	{
	public:
		virtual void Serialize(const AssetMetaData& metadata, const Ref<Asset>& asset) const override;
		virtual bool TryLoadData(const AssetMetaData& metadata, Ref<Asset>& asset) const override;
	};

	class StaticMeshSerializer : public AssetSerializer
	{
	public:
		virtual void Serialize(const AssetMetaData& metadata, const Ref<Asset>& asset) const override;
		virtual bool TryLoadData(const AssetMetaData& metadata, Ref<Asset>& asset) const override;
	};

}