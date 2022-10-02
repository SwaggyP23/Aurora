#include "Aurorapch.h"
#include "Scene.h"
#include "Entity.h"

#include "Components.h"
#include "ScriptableEntity.h"
#include "Renderer/Renderer.h"
#include "Editor/EditorResources.h"

#include <glm/gtc/type_ptr.hpp>

namespace Aurora {

	Ref<Scene> Scene::Create(const std::string& debugName)
	{
		return CreateRef<Scene>(debugName);
	}

	Scene::Scene(const std::string& debugName)
		: m_Name(debugName)
	{
	}

	Scene::~Scene()
	{
	}

	template<typename T>
	static void CopyComponent(entt::registry& dstRegistry, entt::registry& srcRegistry, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		auto srcEntites = srcRegistry.view<T>();
		for (auto srcEntity : srcEntites)
		{

			entt::entity destEntity = enttMap.at(srcRegistry.get<IDComponent>(srcEntity).ID);

			auto& srcComponent = srcRegistry.get<T>(srcEntity);
			auto& destComponent = dstRegistry.emplace_or_replace<T>(destEntity, srcComponent);
		}
	}

	void Scene::CopyTo(Ref<Scene> target)
	{
		target->m_Name = m_Name;

		target->m_Environment = m_Environment;
		target->m_EnvironmentIntensity = m_EnvironmentIntensity;
		target->m_EnvironmentLOD = m_EnvironmentLOD;

		entt::registry& srcSceneRegistry = m_Registry;
		entt::registry& dstSceneRegistry = target->m_Registry;
		std::unordered_map<UUID, entt::entity> enttMap;

		auto idView = srcSceneRegistry.view<IDComponent>();
		for (auto e : idView)
		{
			UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;
			const std::string& name = srcSceneRegistry.get<TagComponent>(e).Tag;

			Entity newEntity = target->CreateEntityWithUUID(uuid, name);
			enttMap[uuid] = (entt::entity)newEntity;
		}

		CopyComponent<TagComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<TransformComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<StaticMeshComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<SpriteRendererComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<CameraComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<NativeScriptComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<SkyLightComponent>(target->m_Registry, m_Registry, enttMap);

		target->m_ViewportWidth = m_ViewportWidth;
		target->m_ViewportHeight = m_ViewportHeight;
	}

	Entity Scene::CreateEntityWithUUID(UUID id, const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<IDComponent>(id);
		entity.AddComponent<TransformComponent>();
		TagComponent& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name == "" ? "AuroraDefault" : std::move(name);

		return entity;
	}

	Entity Scene::CreateEntity(const char* name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Scene::CopyEntity(Entity entity)
	{
		AR_CORE_ASSERT(false);
		AR_CORE_CHECK(false, "This is still not working and needs ALOT of rework!");

		static uint32_t nameIncremet = 1;
		std::string name = entity.GetComponent<TagComponent>().Tag;
		Entity result = CreateEntity(name.c_str());

		glm::vec3 translation, rotation, scale;
		translation = entity.GetComponent<TransformComponent>().Translation;
		rotation = entity.GetComponent<TransformComponent>().Rotation;
		scale = entity.GetComponent<TransformComponent>().Scale;
		result.GetComponent<TransformComponent>().Translation = translation;
		result.GetComponent<TransformComponent>().Rotation = rotation;
		result.GetComponent<TransformComponent>().Scale = scale;

		if (entity.HasComponent<SpriteRendererComponent>())
		{
			glm::vec4 color = entity.GetComponent<SpriteRendererComponent>().Color;
			result.AddComponent<SpriteRendererComponent>(color);
		}

		if (entity.HasComponent<CameraComponent>())
		{
			CameraComponent cc = entity.GetComponent<CameraComponent>();
			result.AddComponent<CameraComponent>(cc);
		}

		return result;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	void Scene::Clear()
	{
		m_Registry.clear();
	}

	void Scene::OnRenderEditor(Ref<SceneRenderer> renderer, TimeStep ts, const EditorCamera& camera)
	{
		{
			auto skyLightView = m_Registry.view<TransformComponent, SkyLightComponent>();
			if (skyLightView.empty())
				m_Environment = Renderer::GetBlackEnvironment();

			for (auto entity : skyLightView)
			{
				auto [tranform, skyLight] = skyLightView.get<TransformComponent, SkyLightComponent>(entity);

				if (!skyLight.SceneEnvironment && skyLight.DynamicSky)
				{
					const glm::vec3& TAI = skyLight.TurbidityAzimuthInclination;
					Ref<CubeTexture> preethamEnv = Renderer::CreatePreethamSky(TAI.x, TAI.y, TAI.z);
					skyLight.SceneEnvironment = Environment::Create(preethamEnv, preethamEnv);
				}

				m_Environment = skyLight.SceneEnvironment;
				m_EnvironmentIntensity = skyLight.Intensity;
				m_EnvironmentLOD = skyLight.Level;
			}
		}
		
		renderer->SetScene(this);
		renderer->BeginScene({ camera, camera.GetViewMatrix(), camera.GetNearClip(), camera.GetFarClip(), camera.GetFOV() });

		// Static meshes
		{
			auto meshGroup = m_Registry.view<TransformComponent, StaticMeshComponent>();
			for (auto entity : meshGroup)
			{
				auto [transform, staticMesh] = meshGroup.get<TransformComponent, StaticMeshComponent>(entity);

				if (staticMesh.StaticMesh)
				{
					Entity e = Entity{ entity, this };
					glm::mat4 transform = GetWorldSpaceTransformMatrix(e);

					renderer->SubmitStaticMesh(staticMesh.StaticMesh, staticMesh.MaterialTable, transform, nullptr);
				}
			}
		}

		// End scene here since only meshes are the entities to be rendered and batch renderer is separate
		renderer->EndScene();

		if (renderer->GetFinalPassImage())
		{
			Ref<Renderer2D> renderer2D = renderer->GetRenderer2D();

			// Render only text and debug renderer when that is a thing... and therefore we need to depth test
			renderer2D->BeginScene(camera.GetViewProjection(), camera.GetViewMatrix(), true);
			renderer2D->SetTargetRenderPass(renderer->GetExternalCompositeRenderPass());

			renderer2D->DrawQuad({ 5.0f, 5.0f, -5.0f }, { 10.0f, 5.0f }, { 0.8f, 0.3f, 0.8f, 1.0f });
			renderer2D->DrawQuad({ 0.0f, 0.0f, 0.0f }, { 5.0f, 10.0f }, { 0.2f, 0.3f, 0.8f, 1.0f });
			renderer2D->DrawQuad({ 0.0f, 0.0f, 0.0f }, { 5.0f, 10.0f }, { 0.2f, 0.3f, 0.8f, 1.0f });

			renderer2D->EndScene();
		}
	}

	void Scene::OnRenderRuntime(Ref<SceneRenderer> renderer, TimeStep ts)
	{
		////Update Scripts...
		//{
		//	m_Registry.view<NativeScriptComponent>().each([=](auto entity, NativeScriptComponent& nsc)
		//	{
		//		// TODO: Move to OnScenePlay()... and OnSceneStop() we need to call the OnDestroy for the scripts
		//		if(!nsc.Instance)
		//		{
		//			nsc.Instance = nsc.InstantiateScript();
		//			nsc.Instance->m_Entity = Entity{ entity, this };
		//			nsc.Instance->OnCreate();					
		//		}

		//		nsc.Instance->OnUpdate(ts);
		//	});
		//}

		Entity cameraEntity = GetPrimaryCameraEntity();
		if (!cameraEntity)
			return;

		glm::mat4 cameraViewMatrix = glm::inverse(GetWorldSpaceTransformMatrix(cameraEntity));
		AR_CORE_CHECK(cameraEntity != Entity::nullEntity, "Scene does not contain any cameras!");
		SceneCamera& camera = cameraEntity.GetComponent<CameraComponent>();
		camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);

		{
			auto skyLightView = m_Registry.view<TransformComponent, SkyLightComponent>();
			if (skyLightView.empty())
				m_Environment = Renderer::GetBlackEnvironment();

			for (auto entity : skyLightView)
			{
				auto [tranform, skyLight] = skyLightView.get<TransformComponent, SkyLightComponent>(entity);

				if (!skyLight.SceneEnvironment && skyLight.DynamicSky)
				{
					const glm::vec3& TAI = skyLight.TurbidityAzimuthInclination;
					Ref<CubeTexture> preethamEnv = Renderer::CreatePreethamSky(TAI.x, TAI.y, TAI.z);
					skyLight.SceneEnvironment = Environment::Create(preethamEnv, preethamEnv);
				}

				m_Environment = skyLight.SceneEnvironment;
				m_EnvironmentIntensity = skyLight.Intensity;
				m_EnvironmentLOD = skyLight.Level;
			}
		}

		renderer->SetScene(this);
		renderer->BeginScene({ camera, cameraViewMatrix, camera.GetPerspectiveNearClip(), camera.GetPerspectiveFarClip(), camera.GetDegPerspectiveVerticalFOV() });

		// Static meshes
		{
			auto meshGroup = m_Registry.view<TransformComponent, StaticMeshComponent>();
			for (auto entity : meshGroup)
			{
				auto [transform, staticMesh] = meshGroup.get<TransformComponent, StaticMeshComponent>(entity);

				if (staticMesh.StaticMesh)
				{
					Entity e = Entity{ entity, this };
					glm::mat4 transform = GetWorldSpaceTransformMatrix(e);

					renderer->SubmitStaticMesh(staticMesh.StaticMesh, staticMesh.MaterialTable, transform, nullptr);
				}
			}
		}

		// End scene here since only meshes are the entities to be rendered and batch renderer is separate
		renderer->EndScene();

		if (renderer->GetFinalPassImage())
		{
			Ref<Renderer2D> renderer2D = renderer->GetRenderer2D();

			// Render only text and debug renderer when that is a thing... and therefore we need to depth test
			renderer2D->BeginScene(camera.GetProjection() * cameraViewMatrix, cameraViewMatrix, true);
			renderer2D->SetTargetRenderPass(renderer->GetExternalCompositeRenderPass());

			renderer2D->DrawQuad({ 5.0f, 5.0f, -5.0f }, { 10.0f, 5.0f }, { 0.8f, 0.3f, 0.8f, 1.0f });
			renderer2D->DrawQuad({ 0.0f, 0.0f, 0.0f }, { 5.0f, 10.0f }, { 0.2f, 0.3f, 0.8f, 1.0f });
			renderer2D->DrawQuad({ 0.0f, 0.0f, 0.0f }, { 5.0f, 10.0f }, { 0.2f, 0.3f, 0.8f, 1.0f });

			renderer2D->EndScene();
		}
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		if (m_ViewportWidth == width && m_ViewportHeight == height)
			return;

		m_ViewportWidth = width;
		m_ViewportHeight = height;

		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			cameraComponent.Camera.SetViewportSize(width, height);
		}
	}

	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			const auto& camera = view.get<CameraComponent>(entity);

			if (camera.Primary)
			{
				AR_CORE_ASSERT(camera.Camera.GetOrthographicSize() || camera.Camera.GetDegPerspectiveVerticalFOV(), "Camera is not fully initialized!");
				return Entity{ entity, this };
			}
		}

		return Entity::nullEntity;
	}

	glm::mat4 Scene::GetWorldSpaceTransformMatrix(Entity entity)
	{
		glm::mat4 transform = glm::mat4(1.0f);

		return transform * entity.Transform().GetTransform();
	}

}