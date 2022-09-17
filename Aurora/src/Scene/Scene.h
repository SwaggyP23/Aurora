#pragma once

#include "Core/TimeStep.h"
#include "Core/UUID.h"
#include "Editor/EditorCamera.h"

#include "Graphics/Shader.h" // TODO: Temp...
#include "Graphics/Model.h" // TODO: Temp...
#include "Graphics/CubeTexture.h" // TODO: Temp...

#include <entt/entt.hpp>

namespace Aurora {

	class Entity;

	class Scene : public RefCountedObject
	{
	public:
		Scene(const std::string& debugName);
		~Scene();

		static Ref<Scene> Create(const std::string& debugName = "Scene");

		Entity CreateEntityWithUUID(UUID id, const std::string& name = "");
		Entity CreateEntity(const char* name = "");
		Entity CopyEntity(Entity entity);
		void DestroyEntity(Entity entity);
		void Clear();
		inline size_t Size() const { return m_Registry.size(); }

		void OnUpdateEditor(TimeStep ts, const EditorCamera& camera, glm::vec4 puh, glm::mat4 trans); // TODO: TEMPORARY!!!!!!!!!
		void OnUpdateRuntime(TimeStep ts);
		void OnViewportResize(uint32_t width, uint32_t height);

		// This a conveniance function just in case
		Entity GetPrimaryCameraEntity();

		template<typename... Args>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<Args...>();
		}

		inline const std::string& GetName() const { return m_Name; }
		inline std::string& GetName() { return m_Name; }

	private:
		std::string m_Name = "Untitled Scene";

		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		friend class Entity;
		friend class EditorLayer; // Should be SceneHierarchyPanel once they are split up into separate classes
		friend class SceneSerializer;

	};

}