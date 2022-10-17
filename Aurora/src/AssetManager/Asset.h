#pragma once

#include "Core/Base.h"
#include "Core/UUID.h"
#include "AssetTypes.h"

namespace Aurora {

	using AssetHandle = UUID;

	class Asset : public RefCountedObject
	{
	public:
		Asset() = default;
		virtual ~Asset() {}

		static AssetType GetStaticType() { return AssetType::None; }
		virtual AssetType GetAssetType() const { return AssetType::None; }

		bool IsValid() const { return ((Flags & (uint16_t)AssetFlag::Missing) | (Flags & (uint16_t)AssetFlag::Invalid)) == 0; }

		virtual bool operator==(const Asset& other)
		{
			return Handle == other.Handle;
		}

		virtual bool operator!=(const Asset& other)
		{
			return !(*this == other);
		}

		bool IsFlagSet(AssetFlag flag) const { return (uint16_t)flag & Flags; }
		void SetFlag(AssetFlag flag, bool value = true) // Same api as in material
		{
			if (value)
				Flags |= (uint16_t)flag;
			else
				Flags &= ~(uint16_t)flag;
		}

	public:
		AssetHandle Handle = 0;
		uint16_t Flags = (uint16_t)AssetFlag::None;

	};

	inline static std::unordered_map<std::string, AssetType> s_AssetExtensionsMap =
	{
		// Textures
		{ ".png", AssetType::Texture        },
		{ ".jpg", AssetType::Texture        },
		{ ".jpeg", AssetType::Texture       },
		{ ".hdr", AssetType::EnvironmentMap },

		// Mesh
		{ ".asmesh", AssetType::StaticMesh  },
		{ ".fbx", AssetType::MeshSource     },
		{ ".gltf", AssetType::MeshSource    },
		{ ".glb", AssetType::MeshSource     },
		{ ".obj", AssetType::MeshSource     },
		{ ".dae", AssetType::MeshSource     }, // TODO: May be removed later

		// Materials
		{ ".amaterial", AssetType::Material },

		// Scene
		{ ".ascene", AssetType::Scene       }, // TODO: Change scene file extension to be .ascene instead of .aurora

		// Fonts
		{ ".afa", AssetType::Font           }, // TODO: Maybe change to be a texture since it is technically a texture?
		{ ".ttf", AssetType::Font           },
		{ ".ttc", AssetType::Font           },
		{ ".otf", AssetType::Font           },
	};

}