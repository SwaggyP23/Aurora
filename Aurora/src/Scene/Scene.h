#pragma once

#include "Core/Base.h"
#include "Core/TimeStep.h"
#include "Core/UUID.h"
#include "Editor/EditorCamera.h"
#include "Graphics/SceneEnvironment.h"

#include <entt/entt.hpp>

namespace Aurora {

	class Entity;
	class SceneRenderer;

	class Scene : public RefCountedObject
	{
	public:
		Scene(const std::string& debugName);
		virtual ~Scene();

		[[nodiscard]] static Ref<Scene> Create(const std::string& debugName = "Scene");

		void CopyTo(Ref<Scene> target);

		Entity CreateEntityWithUUID(UUID id, const std::string& name = "");
		Entity CreateEntity(const char* name = "");
		Entity CopyEntity(Entity entity);
		void DestroyEntity(Entity entity);
		void Clear();
		[[nodiscard]] inline size_t Size() const { return m_Registry.size(); }

		void OnRenderEditor(Ref<SceneRenderer> renderer, TimeStep ts, const EditorCamera& camera);
		void OnRenderRuntime(Ref<SceneRenderer> renderer, TimeStep ts);

		void OnViewportResize(uint32_t width, uint32_t height);

		// This a conveniance function just in case
		[[nodiscard]] Entity GetPrimaryCameraEntity();

		[[nodiscard]] glm::mat4 GetWorldSpaceTransformMatrix(Entity entity);

		template<typename... Args>
		[[nodiscard]]
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<Args...>();
		}

		[[nodiscard]] inline const std::string& GetName() const { return m_Name; }
		[[nodiscard]] inline std::string& GetName() { return m_Name; }

	private:
		std::string m_Name = "Untitled Scene";

		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;

		Ref<Environment> m_Environment = nullptr;
		float m_EnvironmentIntensity = 1.0f;
		float m_EnvironmentLOD = 0.0f;

		friend class Entity;
		friend class EditorLayer; // Should be SceneHierarchyPanel once they are split up into separate classes
		friend class SceneSerializer;
		friend class SceneRenderer;

	};

}