#pragma once

#include "Core/Base.h"
#include "Texture.h"

namespace Aurora {

	struct Environment : public RefCountedObject
	{
		constexpr Environment() = default;
		Environment(const Ref<CubeTexture>& radianceMap, const Ref<CubeTexture>& irradianceMap)
			: RadianceMap(radianceMap), IrradianceMap(irradianceMap) {}
		~Environment() = default;

		static Ref<Environment> Create(const Ref<CubeTexture>& radianceMap, const Ref<CubeTexture>& irradianceMap);

		Ref<CubeTexture> RadianceMap;
		Ref<CubeTexture> IrradianceMap;
	};

}