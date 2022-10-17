#pragma once

#include "Asset.h"

#include <filesystem>

namespace Aurora {

	struct AssetMetaData
	{
		AssetHandle Handle = 0;
		AssetType Type = AssetType::None;

		std::filesystem::path FilePath;
		bool IsDataLoaded = false;
		bool IsMemoryAsset = false; // TODO: What is this????

		bool IsValid() const { return Handle != 0 && !IsMemoryAsset; }
	};

}