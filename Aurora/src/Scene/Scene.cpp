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

	void Scene::OnUpdate(TimeStep ts)
	{
		//Update Scripts...
		{
			m_Registry.view<NativeScriptComponent>().each([=](auto entity, NativeScriptComponent& nsc)
			{
				// TODO: Move to OnScenePlay()... and OnSceneStop() we need to call the OnDestroy for the scripts
				if(!nsc.Instance)
				{
					nsc.Instance = nsc.InstantiateScript();
					nsc.Instance->m_Entity = Entity{ entity, this };
					nsc.Instance->OnCreate();					
				}

				nsc.Instance->OnUpdate(ts);
			});
		}

		Camera* mainCamera = nullptr;
		glm::mat4* mainTransform = nullptr;
		{
			auto view = m_Registry.view<TransformComponent, CameraComponent>();
			for (auto entity : view)
			{
				auto[transform, camera] = view.get<TransformComponent, CameraComponent>(entity);

				if (camera.Primary)
				{
					mainCamera = &camera.Camera;
					mainTransform = &(glm::translate(glm::mat4(1.0f), transform.Translation) * glm::scale(glm::mat4(1.0f), transform.Scale));
				}
			}
		}

		if (mainCamera)
		{
			Renderer3D::BeginScene(mainCamera->GetProjection(), *mainTransform);

			auto view = m_Registry.view<TransformComponent, SpriteRendererComponent>();
			for (auto entity : view)
			{
				auto[transform, sprite] = view.get<TransformComponent, SpriteRendererComponent>(entity);

				Renderer3D::DrawQuad(transform.Translation, transform.Scale, sprite.Color);
			}

			Renderer3D::EndScene();
		}
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		// Resize the non-fixed aspect ratio cameras...
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (!cameraComponent.FixedAspectRatio)
			{
				cameraComponent.Camera.SetViewportSize(width, height);
			}

		}
	}

}