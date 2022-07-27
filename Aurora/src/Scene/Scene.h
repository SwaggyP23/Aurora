#pragma once

#include "Core/TimeStep.h"
#include "Graphics/EditorCamera.h"

#include <entt/entt.hpp>

namespace Aurora {

	class Entity;

	class Scene
	{
	public:
		Scene();
		~Scene();

		static Ref<Scene> Create();

		Entity CreateEntity(const char* name = "");
		void DestroyEntity(Entity entity);

		void OnUpdateEditor(TimeStep ts, EditorCamera& camera);
		void OnUpdateRuntime(TimeStep ts);
		void OnViewportResize(uint32_t width, uint32_t height);

		// This a conveniance function just in case
		Entity GetPrimaryCameraEntity();

	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component); // If we have custom components this will not work since it will need the proper overload which is not provided

	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		friend class Entity;
		friend class EditorLayer;
		friend class SceneSerializer;

	};

}