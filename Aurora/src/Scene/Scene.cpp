#include "Aurorapch.h"
#include "Scene.h"

#include "Components.h"

namespace Aurora {

	Scene::Scene()
	{
		struct TransformComponent
		{
			glm::vec3 translation, rotation, scale;

			operator glm::vec3& () { return translation; }
		};

		entt::entity Entity = m_Registry.create();

		m_Registry.emplace<TransformComponent>(Entity);

		TransformComponent& transform = m_Registry.get<TransformComponent>(Entity);
	}

	Scene::~Scene()
	{
	}

	Ref<Scene> Scene::Create()
	{
		return CreateRef<Scene>();
	}

	Entity Scene::CreateEntity(const char* name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name == "" ? "Entity" : name;

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	void Scene::onUpdate(TimeStep ts)
	{
		auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for (auto entity : group)
		{
			auto&[transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

			Renderer3D::DrawQuad(transform.Translation, transform.Scale, sprite.Color);
		}
	}

}