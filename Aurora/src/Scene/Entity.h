#pragma once

// This should be changed from taking a plain old pointer to taking some sort of a weak pointer when i create it.

#include "Core/UUID.h"
#include "Scene.h"
#include "Components.h"

#include <entt/entt.hpp>

namespace Aurora {

	class Entity
	{
	private:
		using nulltype = entt::internal::null;

	public:
		inline static constexpr entt::internal::null nullEntity = {};

	public:
		constexpr Entity() noexcept = default;
		constexpr Entity(nulltype) noexcept
		: m_EntityHandle(nullEntity) {}
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity& other) = default;
		~Entity();

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			AR_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
			T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);

			return component;
		}

		template<typename T>
		[[nodiscard]]
		T& GetComponent()
		{
			AR_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");

			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		void RemoveComponent()
		{
			AR_CORE_ASSERT(HasComponent<T>(), "Entity already has component!");

			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

		template<typename T>
		[[nodiscard]]
		bool HasComponent()
		{
			return m_Scene->m_Registry.has<T>(m_EntityHandle);
		}

		[[nodiscard]] UUID GetUUID()
		{
			return GetComponent<IDComponent>().ID;
		}

		[[nodiscard]] TransformComponent& Transform()
		{
			return GetComponent<TransformComponent>();
		}

		[[nodiscard]] const std::string& GetName()
		{ 
			const std::string& name = GetComponent<TagComponent>().Tag;
			return name;
		}

		bool operator==(const Entity& other) const { return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene; }
		bool operator!=(const Entity& other) const { return !(*this == other); }
		bool operator!=(nulltype other) const { return !(m_EntityHandle == other); }

		operator bool() const { return m_EntityHandle != entt::null; }
		operator entt::entity() const { return m_EntityHandle; }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }

	private:
		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene = nullptr;

	};

}