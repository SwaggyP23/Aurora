#pragma once

// TODO: to be added, mesh components by refering to darianopolis on discord

#include "SceneCamera.h"

#include <glm/glm.hpp>

namespace Aurora {

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(std::string tag)
			: Tag(tag) {}
		TagComponent(const TagComponent&) = default;

	};

	struct TransformComponent
	{
		glm::vec3 Translation{ 0.0f }, Rotation{ 0.0f }, Scale{ 1.0f };

		TransformComponent() = default;
		TransformComponent(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale)
			: Translation(translation), Rotation(rotation), Scale(scale) {}
		TransformComponent(const TransformComponent&) = default;

	};

	struct SpriteRendererComponent
	{
		glm::vec4 Color{ 1.0f };

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const glm::vec4& color)
			: Color(color) {}
		SpriteRendererComponent(const SpriteRendererComponent&) = default;

	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool Primary = true; // TODO: This should be in the Scene
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;

	};

	// Forward declaring the class because it is not necessary to include...
	class ScriptableEntity;

	// This is a NATIVE script component in the sense that this will be a C++ script, C# scripts are another type but too early for that
	struct NativeScriptComponent
	{
		ScriptableEntity* Instance = nullptr;

		ScriptableEntity* (*InstantiateScript)();
		void(*DestroyScript)(NativeScriptComponent*); // Takes in a Native Script comp since the lambda in non capturing... so we need to simulate the "this" pointer...

		template<typename T>
		void Bind()
		{
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyScript = [](NativeScriptComponent* nativeScriptComp) { delete nativeScriptComp->Instance; };
			// Could also set instance to nullptr after deleting however not so necessary other than in Debug...
		}
	};

}