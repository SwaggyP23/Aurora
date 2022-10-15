#include "Aurorapch.h"
#include "Scene.h"
#include "Entity.h"

#include "Components.h"
#include "ScriptableEntity.h"
#include "Graphics/Font.h"
#include "Renderer/Renderer.h"
#include "Editor/EditorResources.h"

#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>

namespace Aurora {

	namespace Utils {

		template<typename T>
		static void CopyComponent(entt::registry& dstRegistry, entt::registry& srcRegistry, const std::unordered_map<UUID, entt::entity>& enttMap)
		{
			auto srcEntites = srcRegistry.view<T>();
			for (auto srcEntity : srcEntites)
			{
				entt::entity destEntity = enttMap.at(srcRegistry.get<IDComponent>(srcEntity).ID);

				T& srcComponent = srcRegistry.get<T>(srcEntity);
				T& destComponent = dstRegistry.emplace_or_replace<T>(destEntity, srcComponent);
			}
		}

		template<typename T>
		static void CopyComponent(Entity dstEntity, Entity srcEntity)
		{
			if (srcEntity.HasComponent<T>())
			{
				T& srcComponent = srcEntity.GetComponent<T>();
				T& dstComponent = dstEntity.AddComponent<T>();
				dstComponent = srcComponent;
			}
		}

	}

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

		Utils::CopyComponent<TagComponent>(target->m_Registry, m_Registry, enttMap);
		Utils::CopyComponent<TransformComponent>(target->m_Registry, m_Registry, enttMap);
		Utils::CopyComponent<CameraComponent>(target->m_Registry, m_Registry, enttMap);
		Utils::CopyComponent<StaticMeshComponent>(target->m_Registry, m_Registry, enttMap);
		Utils::CopyComponent<SpriteRendererComponent>(target->m_Registry, m_Registry, enttMap);
		Utils::CopyComponent<CircleRendererComponent>(target->m_Registry, m_Registry, enttMap);
		Utils::CopyComponent<NativeScriptComponent>(target->m_Registry, m_Registry, enttMap);
		Utils::CopyComponent<SkyLightComponent>(target->m_Registry, m_Registry, enttMap);

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

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Scene::CopyEntity(Entity entity)
	{
		const std::string& name = entity.GetComponent<TagComponent>().Tag;
		Entity result = CreateEntity(name.c_str());

		const TransformComponent& srcTc = entity.GetComponent<TransformComponent>();
		TransformComponent& dstTc = result.Transform();
		dstTc = srcTc;

		Utils::CopyComponent<CameraComponent>(result, entity);
		Utils::CopyComponent<StaticMeshComponent>(result, entity);
		Utils::CopyComponent<SpriteRendererComponent>(result, entity);
		Utils::CopyComponent<CircleRendererComponent>(result, entity);
		Utils::CopyComponent<NativeScriptComponent>(result, entity);
		Utils::CopyComponent<SkyLightComponent>(result, entity);

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

	void Scene::OnRenderEditor(Ref<SceneRenderer> renderer, TimeStep ts, const EditorCamera& camera, glm::vec3& albedo, glm::vec3& controls)
	{
		// Lights...
		{
			m_LightEnvironment = LightEnvironment();

			// Directional Lights...
			{
				auto dirLightView = m_Registry.view<TransformComponent, DirectionalLightComponent>();
				uint32_t directionalLightIndex = 0;
				for (auto entity : dirLightView)
				{
					auto [transform, dirLight] = dirLightView.get<TransformComponent, DirectionalLightComponent>(entity);

					glm::vec3 direction = -glm::normalize(glm::mat3(transform.GetTransform()) * glm::vec3(1.0f));
					m_LightEnvironment.DirectionalLights[directionalLightIndex++] =
					{
						direction,
						dirLight.Radiance,
						dirLight.Intensity
					};
				}
			}
		}

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
		renderer->BeginScene({ camera, camera.GetViewMatrix(), camera.GetNearClip(), camera.GetFarClip(), camera.GetFOV() }, albedo, controls);

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
			renderer2D->BeginScene(camera.GetViewProjection(), camera.GetViewMatrix());
			renderer2D->SetTargetRenderPass(renderer->GetExternalCompositeRenderPass());

			// TODO: These are only here for testing
			renderer2D->DrawQuad({ 10.0f, -5.0f, 2.0f }, { 5.0f, 10.0f }, { 0.3f, 0.45f, 0.8f, 1.0f });

			AABB aabb = { {-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f} };
			renderer2D->DrawAABB(aabb, glm::mat4(1.0f), { 0.3f, 0.8f, 0.2f, 1.0f });
			renderer2D->DrawRotatedRect({ 0.0f, -10.0f, 0.0f }, glm::vec3(0.0f), { 25.0f, 5.0f }, { 0.0f, 0.0f, 1.0f, 1.0f });

			renderer2D->DrawCircle({ 0.0f, 0.0f, 0.0f }, glm::vec3(0.0f), 5.0f, { 1.0f, 0.0f, 1.0f, 1.0f });
			renderer2D->FillCircle({ 0.0f, 0.0f, 10.0f }, 10.0f, { 1.0f, 0.0f, 0.0f, 1.0f }, 1.0f);

			std::string text = "Hello my name is Reda, nicknamed SwaggyP!";
			renderer2D->DrawString(text, glm::vec3(0.0f, 0.0f, -5.0f), 10.0f, glm::vec4(1.0f, 1.0f, 1.0f, 0.7f));

			// Render text...
			auto textGroup = m_Registry.view<TransformComponent, TextComponent>();
			for (auto entity : textGroup)
			{
				auto [transform, text] = textGroup.get<TransformComponent, TextComponent>(entity);

				Entity e = Entity{ entity, this };
				Ref<Font> font = Font::GetFontAssetForTextComponent(text);
				renderer2D->DrawString(text.TextString, font, GetWorldSpaceTransformMatrix(e), text.MaxWidth, text.Color, text.LineSpacing, text.Kerning);
			}

			renderer2D->EndScene();
		}
	}

	void Scene::OnRenderRuntime(Ref<SceneRenderer> renderer, TimeStep ts)
	{
		// Update scripts
		{
			m_Registry.view<NativeScriptComponent>().each([=](auto entity, NativeScriptComponent& nsc)
			{
				nsc.Instance->OnUpdate(ts);
			});
		}

		Entity cameraEntity = GetPrimaryCameraEntity();
		if (!cameraEntity)
			return;

		glm::mat4 cameraViewMatrix = glm::inverse(GetWorldSpaceTransformMatrix(cameraEntity));
		AR_CORE_CHECK(cameraEntity != Entity::nullEntity, "Scene does not contain any cameras!");
		SceneCamera& camera = cameraEntity.GetComponent<CameraComponent>();
		camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);

		// Lights...
		{
			m_LightEnvironment = LightEnvironment();

			// Directional Lights...
			{
				auto dirLightView = m_Registry.view<TransformComponent, DirectionalLightComponent>();
				uint32_t directionalLightIndex = 0;
				for (auto entity : dirLightView)
				{
					auto [transform, dirLight] = dirLightView.get<TransformComponent, DirectionalLightComponent>(entity);

					glm::vec3 direction = -glm::normalize(glm::mat3(transform.GetTransform()) * glm::vec3(1.0f));
					m_LightEnvironment.DirectionalLights[directionalLightIndex++] =
					{
						direction,
						dirLight.Radiance,
						dirLight.Intensity
					};
				}
			}
		}

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
		renderer->BeginScene({ camera, cameraViewMatrix, camera.GetPerspectiveNearClip(), camera.GetPerspectiveFarClip(), camera.GetDegPerspectiveVerticalFOV() }, glm::vec3(1.0f), glm::vec3(1.0f));

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

			//// TODO: These are only here for testing
			renderer2D->DrawQuad({ 10.0f, -5.0f, 2.0f }, { 5.0f, 10.0f }, { 0.3f, 0.45f, 0.8f, 1.0f });

			AABB aabb = { {-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f} };
			renderer2D->DrawAABB(aabb, glm::mat4(1.0f), { 0.3f, 0.8f, 0.2f, 1.0f });
			renderer2D->DrawRotatedRect({ 0.0f, -10.0f, 0.0f }, glm::vec3(0.0f), { 25.0f, 5.0f }, { 0.0f, 0.0f, 1.0f, 1.0f });

			renderer2D->DrawCircle({ 0.0f, 0.0f, 0.0f }, glm::vec3(0.0f), 5.0f, { 1.0f, 0.0f, 1.0f, 1.0f });
			renderer2D->FillCircle({ 0.0f, 0.0f, 10.0f }, 10.0f, { 1.0f, 0.0f, 0.0f, 1.0f }, 1.0f);

			// Render text...
			auto textGroup = m_Registry.view<TransformComponent, TextComponent>();
			for (auto entity : textGroup)
			{
				auto [transform, text] = textGroup.get<TransformComponent, TextComponent>(entity);

				Entity e = Entity{ entity, this };
				Ref<Font> font = Font::GetFontAssetForTextComponent(text);
				renderer2D->DrawString(text.TextString, font, GetWorldSpaceTransformMatrix(e), text.MaxWidth, text.Color, text.LineSpacing, text.Kerning);
			}

			renderer2D->EndScene();
		}
	}

	void Scene::OnRuntimeStart()
	{
		// Instantiate scripts
		{
			m_Registry.view<NativeScriptComponent>().each([=](auto entity, NativeScriptComponent& nsc)
			{
				if (!nsc.Instance)
				{
					nsc.Instance = nsc.InstantiateScript();
					nsc.Instance->m_Entity = Entity{ entity, this };
					nsc.Instance->OnCreate();
				}
			});
		}
	}

	void Scene::OnRuntimeStop()
	{
		// Destroy scripts
		{
			m_Registry.view<NativeScriptComponent>().each([=](auto entity, NativeScriptComponent& nsc)
			{
				if (nsc.Instance)
					nsc.Instance->OnDestroy();
			});
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

	Entity Scene::GetEntityByName(const std::string& name)
	{
		auto view = m_Registry.view<TagComponent>();
		for (auto entity : view)
		{
			if (m_Registry.get<TagComponent>(entity).Tag == name)
			{
				return Entity{ entity, this };
			}
		}

		AR_CORE_ASSERT(false);
		return {};
	}

}