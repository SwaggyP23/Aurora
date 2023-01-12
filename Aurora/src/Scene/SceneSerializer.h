#pragma once

#include "Scene.h"

namespace Aurora {

	class SceneSerializer
	{
	public:
		static void Serialize(const std::string& filepath, Ref<Scene> scene);
		static bool DeSerialize(const std::string& filepath, Ref<Scene> scene);

	public:
		inline static std::string_view SceneExtension = ".ascene";

	};

}