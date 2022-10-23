#include "Aurorapch.h"
#include "Scene.h"
#include "Entity.h"

#include "Components.h"
#include "ScriptableEntity.h"
#include "AssetManager/AssetManager.h"
#include "Graphics/Font.h"
#include "Renderer/Renderer.h"
#include "Renderer/SceneRenderer.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/DebugRenderer.h"
#include "Editor/EditorResources.h"

#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>

namespace Aurora {

	namespace Utils {

		// For duplicate Entity
		template<typename T>
		static void CopyComponentIfExists(Entity dstEntity, Entity srcEntity)
		{
			if (srcEntity.HasComponent<T>())
			{
				T& srcComponent = srcEntity.GetComponent<T>();
				T& dstComponent = dstEntity.AddComponent<T>();
				dstComponent = srcComponent;
			}
		}

		template<typename... TComponent>
		static void CopyComponentIfExists(Entity dst, Entity src)
		{
			([&]()
			{
				if (src.HasComponent<TComponent>())
				{
					dst.AddOrReplaceComponent<TComponent>(src.GetComponent<TComponent>());
				}
			}(), ...);
		}

		template<typename... TComponent>
		static void CopyComponentIfExists(ComponentGroup<TComponent...>, Entity dst, Entity src)
		{
			CopyComponentIfExists<TComponent...>(dst, src);
		}

		// For copying scene
		template<typename T>
		static void CopyComponentScene(entt::registry& dstRegistry, entt::registry& srcRegistry, const std::unordered_map<UUID, entt::entity>& enttMap)
		{
			auto srcEntites = srcRegistry.view<T>();
			for (auto srcEntity : srcEntites)
			{
				entt::entity destEntity = enttMap.at(srcRegistry.get<IDComponent>(srcEntity).ID);

				T& srcComponent = srcRegistry.get<T>(srcEntity);
				T& destComponent = dstRegistry.emplace_or_replace<T>(destEntity, srcComponent);
			}
		}

		template<typename... TComponent>
		static void CopyComponentVariadicScene(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
		{
			([&]()
			{
				auto view = src.view<TComponent>();
				for (auto srcEntity : view)
				{
					entt::entity dstEntity = enttMap.at(src.get<IDComponent>(srcEntity).ID);

					auto& srcComponent = src.get<TComponent>(srcEntity);
					dst.emplace_or_replace<TComponent>(dstEntity, srcComponent);
				}
			}(), ...);
		}

		template<typename... TComponent>
		static void CopyComponentVariadicScene(ComponentGroup<TComponent...>, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
		{
			CopyComponentVariadicScene<TComponent...>(dst, src, enttMap);
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

		//Utils::CopyComponent<TagComponent>(target->m_Registry, m_Registry, enttMap);
		//Utils::CopyComponent<TransformComponent>(target->m_Registry, m_Registry, enttMap);
		//Utils::CopyComponent<CameraComponent>(target->m_Registry, m_Registry, enttMap);
		//Utils::CopyComponent<StaticMeshComponent>(target->m_Registry, m_Registry, enttMap);
		//Utils::CopyComponent<SpriteRendererComponent>(target->m_Registry, m_Registry, enttMap);
		//Utils::CopyComponent<CircleRendererComponent>(target->m_Registry, m_Registry, enttMap);
		//Utils::CopyComponent<NativeScriptComponent>(target->m_Registry, m_Registry, enttMap);
		//Utils::CopyComponent<SkyLightComponent>(target->m_Registry, m_Registry, enttMap);
		//Utils::CopyComponent<TextComponent>(target->m_Registry, m_Registry, enttMap);

		Utils::CopyComponentVariadicScene(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);

		target->SortEntites();

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

		// The ID should not exist inside the map!
		AR_CORE_ASSERT(m_EntityIDMap.find(id) == m_EntityIDMap.end());
		m_EntityIDMap[id] = entity;

		SortEntites();

		return entity;
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Scene::CopyEntity(Entity entity)
	{
		const std::string& name = entity.GetComponent<TagComponent>().Tag;
		Entity dstEntity = CreateEntity(name.c_str());

		//Utils::CopyComponentIfExists<TransformComponent>(result, entity);
		//Utils::CopyComponentIfExists<CameraComponent>(result, entity);
		//Utils::CopyComponentIfExists<SpriteRendererComponent>(result, entity);
		//Utils::CopyComponentIfExists<CircleRendererComponent>(result, entity);
		//Utils::CopyComponentIfExists<StaticMeshComponent>(result, entity);
		//Utils::CopyComponentIfExists<NativeScriptComponent>(result, entity);
		//Utils::CopyComponentIfExists<DirectionalLightComponent>(result, entity);
		//Utils::CopyComponentIfExists<SkyLightComponent>(result, entity);
		//Utils::CopyComponentIfExists<TextComponent>(result, entity);

		Utils::CopyComponentIfExists(AllComponents{}, dstEntity, entity);

		return dstEntity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		if (m_OnEntityDestroyedCallback)
			m_OnEntityDestroyedCallback(entity);

		UUID id = entity.GetUUID();
		m_Registry.destroy(entity.m_EntityHandle);
		m_EntityIDMap.erase(id);

		SortEntites();
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

				if (!AssetManager::IsAssetHandleValid(skyLight.SceneEnvironment) && skyLight.DynamicSky)
				{
					const glm::vec3& TAI = skyLight.TurbidityAzimuthInclination;
					Ref<CubeTexture> preethamEnv = Renderer::CreatePreethamSky(TAI.x, TAI.y, TAI.z);
					skyLight.SceneEnvironment = AssetManager::CreateMemoryOnlyAsset<Environment>(preethamEnv, preethamEnv);
				}

				m_Environment = AssetManager::GetAsset<Environment>(skyLight.SceneEnvironment);
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
				auto [transform, staticMeshComp] = meshGroup.get<TransformComponent, StaticMeshComponent>(entity);

				if (AssetManager::IsAssetHandleValid(staticMeshComp.StaticMesh))
				{
					auto staticMesh = AssetManager::GetAsset<StaticMesh>(staticMeshComp.StaticMesh);
					if (staticMesh && !staticMesh->IsFlagSet(AssetFlag::Missing))
					{
						Entity e = Entity{ entity, this };
						glm::mat4 transform = GetWorldSpaceTransformMatrix(e);

						renderer->SubmitStaticMesh(staticMesh, staticMeshComp.MaterialTable, transform, nullptr);
					}
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
			
			// Render text...
			{
				auto textGroup = m_Registry.view<TransformComponent, TextComponent>();
				for (auto entity : textGroup)
				{
					auto [transform, text] = textGroup.get<TransformComponent, TextComponent>(entity);

					Entity e = Entity{ entity, this };
					Ref<Font> font = Font::GetFontAssetForTextComponent(text);
					renderer2D->DrawString(text.TextString, font, GetWorldSpaceTransformMatrix(e), text.MaxWidth, text.Color, text.LineSpacing, text.Kerning/*, text.OutLineWidth, text.OutLineColor*/);
				}
			}

			// Save the lineWidth in case debug renderer changed it
			float lineWidth = renderer2D->GetLineWidth();

			// Debug Renderer...
			{
				Ref<DebugRenderer> debugRenderer = renderer->GetDebugRenderer();

				DebugRenderer::RenderQueue& renderQueue = debugRenderer->GetRenderQueue();
				for (DebugRenderer::RenderQueueFunction& func : renderQueue)
					func(renderer2D);

				debugRenderer->ClearRenderQueue();
			}

			renderer2D->EndScene();

			// Restore the lineWidth in case debug renderer changed it
			renderer2D->SetLineWidth(lineWidth);
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

				if (!AssetManager::IsAssetHandleValid(skyLight.SceneEnvironment) && skyLight.DynamicSky)
				{
					const glm::vec3& TAI = skyLight.TurbidityAzimuthInclination;
					Ref<CubeTexture> preethamEnv = Renderer::CreatePreethamSky(TAI.x, TAI.y, TAI.z);
					skyLight.SceneEnvironment = AssetManager::CreateMemoryOnlyAsset<Environment>(preethamEnv, preethamEnv);
				}

				m_Environment = AssetManager::GetAsset<Environment>(skyLight.SceneEnvironment);
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
				auto [transform, staticMeshComp] = meshGroup.get<TransformComponent, StaticMeshComponent>(entity);

				if (AssetManager::IsAssetHandleValid(staticMeshComp.StaticMesh))
				{
					auto staticMesh = AssetManager::GetAsset<StaticMesh>(staticMeshComp.StaticMesh);
					if (staticMesh && !staticMesh->IsFlagSet(AssetFlag::Missing))
					{
						Entity e = Entity{ entity, this };
						glm::mat4 transform = GetWorldSpaceTransformMatrix(e);

						renderer->SubmitStaticMesh(staticMesh, staticMeshComp.MaterialTable, transform, nullptr);
					}
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
			{
				auto textGroup = m_Registry.view<TransformComponent, TextComponent>();
				for (auto entity : textGroup)
				{
					auto [transform, text] = textGroup.get<TransformComponent, TextComponent>(entity);

					Entity e = Entity{ entity, this };
					Ref<Font> font = Font::GetFontAssetForTextComponent(text);
					renderer2D->DrawString(text.TextString, font, GetWorldSpaceTransformMatrix(e), text.MaxWidth, text.Color, text.LineSpacing, text.Kerning);
				}
			}

			// Save the lineWidth in case debug renderer changed it
			float lineWidth = renderer2D->GetLineWidth();

			// Debug Renderer...
			{
				Ref<DebugRenderer> debugRenderer = renderer->GetDebugRenderer();

				DebugRenderer::RenderQueue& renderQueue = debugRenderer->GetRenderQueue();
				for (DebugRenderer::RenderQueueFunction& func : renderQueue)
					func(renderer2D);

				debugRenderer->ClearRenderQueue();
			}

			renderer2D->EndScene();

			// Restore the lineWidth in case debug renderer changed it
			renderer2D->SetLineWidth(lineWidth);
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
			const CameraComponent& camera = view.get<CameraComponent>(entity);

			if (camera.Primary)
			{
				AR_CORE_ASSERT(camera.Camera.GetOrthographicSize() || camera.Camera.GetDegPerspectiveVerticalFOV(), "Camera is not fully initialized!");
				return Entity{ entity, this };
			}
		}

		return Entity::nullEntity;
	}

	glm::mat4 Scene::GetWorldSpaceTransformMatrix(Entity entity) const
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
		return Entity{};
	}

	Entity Scene::TryGetEntityByName(const std::string& name)
	{
		auto view = m_Registry.view<TagComponent>();
		for (auto entity : view)
		{
			if (m_Registry.get<TagComponent>(entity).Tag == name)
			{
				return Entity{ entity, this };
			}
		}

		return Entity{};
	}

	Entity Scene::GetEntityWithUUID(UUID id) const
	{
		AR_PROFILE_FUNCTION();

		AR_CORE_CHECK(m_EntityIDMap.find(id) != m_EntityIDMap.end(), "Entity does not exist in this scene!");
		return m_EntityIDMap.at(id);
	}

	Entity Scene::TryGetEntityWithUUID(UUID id) const
	{
		AR_PROFILE_FUNCTION();

		const auto itr = m_EntityIDMap.find(id);
		if (itr != m_EntityIDMap.end())
			return itr->second;

		return Entity{};
	}

	void Scene::SortEntites()
	{
		m_Registry.sort<IDComponent>([&](const IDComponent first, const IDComponent second)
		{
			auto firstEntityItr = m_EntityIDMap.find(first.ID);
			auto secondEntityItr = m_EntityIDMap.find(second.ID);

			return (uint32_t)(firstEntityItr->second) < (uint32_t)(secondEntityItr->second);
		});
	}

}