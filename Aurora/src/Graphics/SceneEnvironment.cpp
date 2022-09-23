#include "Aurorapch.h"
#include "SceneEnvironment.h"

namespace Aurora {

	Ref<Environment> Environment::Create(const Ref<CubeTexture>& radianceMap, const Ref<CubeTexture>& irradianceMap)
	{
		return CreateRef<Environment>(radianceMap, irradianceMap);
	}

}