#pragma once

#include "Scene.h"

namespace Aurora {

	class SceneSerializer
	{
	public:
		SceneSerializer(const Ref<Scene>& scene);

		void SerializeToText(const std::string& filepath);
		void SerializeToBinary(const std::string& filepath);

		bool DeSerializeFromText(const std::string& filepath);
		bool DeSerializeFromBinary(const std::string& filepath);

	public:
		inline static std::string_view SceneExtension = ".aurora"; // TODO: Change to .ascene

	private:
		Ref<Scene> m_Scene;

	};

}