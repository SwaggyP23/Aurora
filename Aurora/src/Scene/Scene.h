#pragma once

#include "AssetManager/Asset.h"
#include "Core/Base.h"
#include "Core/TimeStep.h"
#include "Core/UUID.h"
#include "Editor/EditorCamera.h"
#include "Graphics/SceneEnvironment.h"

#include <entt/entt.hpp>

namespace Aurora {

	struct DirectionalLight
	{
		glm::vec3 Direction = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Radiance = { 0.0f, 0.0f, 0.0f };
		float Intensity = 1.0f;
	};

	struct LightEnvironment
	{
		DirectionalLight DirectionalLights[4]; // Can have 4 however only renders with one
	};

	class Entity;
	class SceneRenderer;

	class Scene : public Asset
	{
	public:
		Scene(const std::string& debugName);
		virtual ~Scene();

		static Ref<Scene> Create(const std::string& debugName = "Scene");

		void OnRenderEditor(Ref<SceneRenderer> renderer, TimeStep ts, const EditorCamera& camera, glm::vec3& albedo, glm::vec3& controls);
		void OnRenderRuntime(Ref<SceneRenderer> renderer, TimeStep ts);

		// Update physics and scripts and all that...
		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnViewportResize(uint32_t width, uint32_t height);

		void CopyTo(Ref<Scene> target);

		void SetEntityDestroyedCallback(const std::function<void(Entity)>& callback) { m_OnEntityDestroyedCallback = callback; }

		Entity CreateEntityWithUUID(UUID id, const std::string& name = "");
		Entity CreateEntity(const std::string& name = "");
		Entity CopyEntity(Entity entity);
		void DestroyEntity(Entity entity);
		void Clear();
		inline size_t Size() const { return m_Registry.size(); }

		// These are conveniance functions just in case
		Entity GetPrimaryCameraEntity();
		Entity GetEntityByName(const std::string& name);
		Entity TryGetEntityByName(const std::string& name);
		Entity GetEntityWithUUID(UUID id) const;
		Entity TryGetEntityWithUUID(UUID id) const;

		glm::mat4 GetWorldSpaceTransformMatrix(Entity entity) const;

		template<typename... Args>
		[[nodiscard]]
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<Args...>();
		}

		inline const std::string& GetName() const { return m_Name; }
		inline std::string& GetName() { return m_Name; }

		static AssetType GetStaticType() { return AssetType::Scene; }
		virtual AssetType GetAssetType() const override { return GetStaticType(); }

	private:
		void SortEntites();

	private:
		std::string m_Name = "Untitled Scene";

		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;

		Ref<Environment> m_Environment = nullptr;
		float m_EnvironmentIntensity = 1.0f;
		float m_EnvironmentLOD = 0.0f;

		LightEnvironment m_LightEnvironment;

		std::map<UUID, Entity> m_EntityIDMap;

		std::function<void(Entity)> m_OnEntityDestroyedCallback;

		friend class Entity;
		friend class EditorLayer; // Should be SceneHierarchyPanel once they are split up into separate classes
		friend class SceneSerializer;
		friend class SceneRenderer;

	};

}