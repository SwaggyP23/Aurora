#pragma once

#include "SceneCamera.h"
#include "Graphics/Material.h"
#include "Graphics/Mesh.h"
#include "Graphics/SceneEnvironment.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Aurora {

	struct IDComponent
	{
		UUID ID;
	};

	struct TagComponent
	{
		std::string Tag;
	};

	// TODO: Edge coloring maybe??
	struct TextComponent
	{
		std::string TextString = "";
		size_t TextHash;

		// Font
		AssetHandle FontHandle;
		glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		float LineSpacing = 0.0f;
		float Kerning = 0.0f;

		// Layout
		float MaxWidth = 10.0f;

		TextComponent() = default;
		TextComponent(const TextComponent&) = default;
	};

	// TODO: Use quaternions for rotation...
	struct TransformComponent
	{
		glm::vec3 Translation{ 0.0f };
		glm::vec3 Rotation{ 0.0f }; // Stored in radians
		glm::vec3 Scale{ 1.0f };

		glm::mat4 GetTransform()
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

			return glm::translate(glm::mat4(1.0f), Translation) * rotation * glm::scale(glm::mat4(1.0f), Scale);
		}
	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool Primary = true;

		CameraComponent() = default;
		CameraComponent(const CameraComponent& other) = default;

		operator SceneCamera& () { return Camera; }
		operator const SceneCamera& () const { return Camera; }
	};

	// TODO: Need to add textures to it and serialize them...
	struct SpriteRendererComponent
	{
		glm::vec4 Color{ 1.0f };
		AssetHandle Texture;
		float TilingFactor = 1.0f;
	};

	struct CircleRendererComponent
	{
		glm::vec4 Color{ 1.0f };
		float Thickness = 1.0f;
	};

	struct StaticMeshComponent
	{
		Ref<StaticMesh> StaticMesh;
		Ref<MaterialTable> MaterialTable = MaterialTable::Create();

		StaticMeshComponent() = default;
		StaticMeshComponent(const StaticMeshComponent& other)
			: StaticMesh(other.StaticMesh), MaterialTable(other.MaterialTable) {}
		StaticMeshComponent(const Ref<Aurora::StaticMesh>& other)
			: StaticMesh(other) {}
	};

	// Forward declaring the class so we dont include it
	class ScriptableEntity;

	// This is a NATIVE script component in the sense that this will be a C++ script, C# scripts are another type but too early for that
	// To add a NativeScriptComponent to your entity use: entity.AddComponent<NativeScriptComponent>().Bind<** YOU SCRIPT COMPONENT **>();
	struct NativeScriptComponent
	{
		ScriptableEntity* Instance = nullptr;

		ScriptableEntity* (*InstantiateScript)();
		void (*DestroyScript)(NativeScriptComponent*); // Takes in a Native Script comp since the lambda in non capturing... so we need to simulate the "this" pointer...

		template<typename T>
		void Bind()
		{
			InstantiateScript = []() { return (ScriptableEntity*)(new T()); };
			DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
		}
	};

	enum class LightType
	{
		None = 0,
		Directional,
		Point
	};

	struct DirectionalLightComponent
	{
		glm::vec3 Radiance = { 1.0f, 1.0f, 1.0f };
		float Intensity = 1.0f; // Multiplier
	};

	struct SkyLightComponent
	{
		AssetHandle SceneEnvironment;
		float Level = 0.5f;
		float Intensity = 1.0f;

		bool DynamicSky = false;
		glm::vec3 TurbidityAzimuthInclination{ 2.0f, 0.0f, 0.0f };
	};

}