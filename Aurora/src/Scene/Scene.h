#pragma once

#include "Core/TimeStep.h"

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

		void OnUpdate(TimeStep ts);
		void OnViewportResize(uint32_t width, uint32_t height);

	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		friend class Entity;

	};

}