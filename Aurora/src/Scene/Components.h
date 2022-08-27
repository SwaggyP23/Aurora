#pragma once

// TODO: to be added, mesh components by refering to darianopolis on discord

#include "SceneCamera.h"
#include "Graphics/Model.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Aurora {

	// TODO: Add the UUID component

	struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
	};

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(std::string tag)
			: Tag(tag) {}
		TagComponent(const TagComponent&) = default;

	};

	// TODO: Use quaternions for rotation...
	struct TransformComponent
	{
		glm::vec3 Translation{ 0.0f }, Rotation{ 0.0f }, Scale{ 1.0f }; // The rotation is stored in radians

		TransformComponent() = default;
		TransformComponent(const glm::vec3& translation, const glm::vec3& rotation = glm::vec3(0.0f), const glm::vec3& scale = glm::vec3(1.0f))
			: Translation(translation), Rotation(rotation), Scale(scale) {}
		TransformComponent(const TransformComponent&) = default;

		glm::mat4 GetTransform()
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

			return glm::translate(glm::mat4(1.0f), Translation) * rotation * glm::scale(glm::mat4(1.0f), Scale);
		}

	};

	// TODO: Rework...
	struct ModelComponent
	{
		Model model;

		ModelComponent() = default;
		ModelComponent(const std::string& filepath)
			: model(filepath) {}
		ModelComponent(const ModelComponent&) = default;

	};

	// TODO: Rework...
	struct SpriteRendererComponent
	{// This should contain a Ref<Material/MaterialInstance> and a shader to that material...(Materials are capable of holding both the shader and data
		glm::vec4 Color{ 1.0f };

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const glm::vec4& color)
			: Color(color) {}
		SpriteRendererComponent(const SpriteRendererComponent&) = default;

	};

	// TODO: Rewoork...!
	struct CameraComponent
	{
		SceneCamera Camera;
		bool Primary = true;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;

	};

	// Forward declaring the class so we dont include it
	class ScriptableEntity;

	// This is a NATIVE script component in the sense that this will be a C++ script, C# scripts are another type but too early for that
	// To add a NativeScriptComponent to your entity use: entity.AddComponent<NativeScriptComponent>().Bind<** YOU SCRIPT COMPONENT **>();
	struct NativeScriptComponent
	{
		ScriptableEntity* Instance = nullptr;

		ScriptableEntity* (*InstantiateScript)();
		void(*DestroyScript)(NativeScriptComponent*); // Takes in a Native Script comp since the lambda in non capturing... so we need to simulate the "this" pointer...

		template<typename T>
		void Bind()
		{
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyScript = [](NativeScriptComponent* nativeScriptComp) { delete nativeScriptComp->Instance; nativeScriptComp->Instance = nullptr; };
		}
	};

}