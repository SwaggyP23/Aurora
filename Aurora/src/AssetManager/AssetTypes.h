#pragma once

#include "Core/Base.h"

#include <string>

namespace Aurora {
	
	enum class AssetFlag : uint16_t
	{
		None = 0,
		Missing = BIT(1),
		Invalid = BIT(2)
	};

	enum class AssetType : uint16_t
	{
		None = 0,
		Texture,
		EnvironmentMap,
		StaticMesh, // This will be used when i start creating my own .Asmesh files where they are compatible with aurora
		MeshSource, // This is for meshSources such as .fbx, .gltf, .dae ...
		Material,
		// TODO: Add physics material when physics exists
		Scene,
		Font
		// TODO: Add script and script files...
	};

	namespace Utils {

		inline AssetType AssetTypeFromString(const std::string& type)
		{
			if (type == "None")				return AssetType::None;
			if (type == "Scene")			return AssetType::Scene;
			if (type == "StaticMesh")		return AssetType::StaticMesh;
			if (type == "MeshSource")		return AssetType::MeshSource;
			if (type == "Material")			return AssetType::Material;
			if (type == "Texture")			return AssetType::Texture;
			if (type == "EnvironmentMap")	return AssetType::EnvironmentMap;
			if (type == "Font")				return AssetType::Font;

			AR_CORE_ASSERT(false, "Unknown Asset Type");
			return AssetType::None;
		}

		inline const char* AssetTypeToString(AssetType type)
		{
			switch (type)
			{
			    case AssetType::None:				return "None";
			    case AssetType::Texture:			return "Texture";
			    case AssetType::EnvironmentMap:		return "EnvironmentMap";
			    case AssetType::StaticMesh:			return "StaticMesh";
			    case AssetType::MeshSource:			return "MeshSource";
			    case AssetType::Material:			return "Material";
			    case AssetType::Scene:				return "Scene";
			    case AssetType::Font:				return "Font";
			}

			AR_CORE_ASSERT(false, "Unkown Asset Type!");
			return "None";
		}

	}

}