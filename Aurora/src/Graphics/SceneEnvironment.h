#pragma once

#include "AssetManager/Asset.h"
#include "Core/Base.h"
#include "Texture.h"

namespace Aurora {

	struct Environment : public Asset
	{
		constexpr Environment() = default;
		Environment(const Ref<CubeTexture>& radianceMap, const Ref<CubeTexture>& irradianceMap)
			: RadianceMap(radianceMap), IrradianceMap(irradianceMap) {}
		virtual ~Environment() = default;

		static Ref<Environment> Create(const Ref<CubeTexture>& radianceMap, const Ref<CubeTexture>& irradianceMap);

		static AssetType GetStaticType() { return AssetType::EnvironmentMap; }
		virtual AssetType GetAssetType() const override { return GetStaticType(); }

		Ref<CubeTexture> RadianceMap;
		Ref<CubeTexture> IrradianceMap;
	};

}