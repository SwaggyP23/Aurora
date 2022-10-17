#pragma once

#include "AssetSerializers.h"

namespace Aurora {

	class AssetImporter
	{
	public:
		static void Init();
		static void Shutdown();

		static void Serialize(const AssetMetaData& metaData, const Ref<Asset>& asset);
		static void Serialize(const Ref<Asset>& asset);
		static bool TryLoadData(const AssetMetaData& metaData, Ref<Asset>& asset);

	private:
		static std::unordered_map<AssetType, Scope<AssetSerializer>> s_Serializers;

	};

}