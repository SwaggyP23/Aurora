#include "Aurorapch.h"
#include "Scene.h"

#include "Components.h"

namespace Aurora {

	Ref<Scene> Scene::Create()
	{
		return CreateRef<Scene>();
	}

	Scene::Scene()
	{
	}

	Scene::~Scene()
	{
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
		Camera* mainCamera = nullptr;
		glm::mat4* mainTransform = nullptr;
		{
			auto group = m_Registry.group<CameraComponent>(entt::get<TransformComponent>);
			for (auto entity : group)
			{
				auto& [transform, camera] = group.get<TransformComponent, CameraComponent>(entity);

				if (camera.Primary)
				{
					mainCamera = &camera.SceneCamera;
					mainTransform = &(glm::translate(glm::mat4(1.0f), transform.Translation) * glm::scale(glm::mat4(1.0f), transform.Scale));
				}
			}
		}

		if (mainCamera)
		{
			Renderer3D::BeginScene(mainCamera->GetProjection(), *mainTransform);

			auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto entity : group)
			{
				auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

				Renderer3D::DrawQuad(transform.Translation, transform.Scale, sprite.Color);
			}

			Renderer3D::EndScene();
		}
	}

}