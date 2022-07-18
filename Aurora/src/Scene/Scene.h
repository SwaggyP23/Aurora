#pragma once

#include "Core/TimeStep.h"

#include <entt/entt.hpp>

namespace Aurora {

	class Entity;

	class Scene
	{
	public:
		Scene(bool dummyVarForRefSystem); // This parameter is to be removed when a good reference counting system is made or do a good soln
		~Scene();

		static Ref<Scene> Create();

		Entity CreateEntity(const char* name = "");
		void DestroyEntity(Entity entity);

		void onUpdate(TimeStep ts);

		// TEMP..
		entt::registry& Reg() { return m_Registry; }

	private:
		entt::registry m_Registry;

		friend class Entity;

	};

}