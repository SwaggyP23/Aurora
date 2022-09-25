#include "Aurorapch.h"
#include "Scene.h"
#include "Entity.h"

#include "Graphics/Model.h"
#include "Graphics/UniformBuffer.h" // TODO: Temp...!
#include "Components.h"
#include "ScriptableEntity.h"
#include "Renderer/Renderer.h"
#include "Renderer/Renderer3D.h"
#include "Editor/EditorResources.h"

#include <glm/gtc/type_ptr.hpp>

namespace Aurora {

	static Ref<UniformBuffer> s_ModelUniBuffer;
	static Ref<Material> s_Mat;
	static Ref<Texture2D> s_Texture;
	static TextureProperties s_Props;
	static Ref<Shader> s_ModelShader;
	static bool s_Created = false;

	Ref<Scene> Scene::Create(const std::string& debugName)
	{
		return CreateRef<Scene>(debugName);
	}

	Scene::Scene(const std::string& debugName)
		: m_Name(debugName)
	{
		if (!s_Created)
		{
			s_Mat = Material::Create("Test Mat", Renderer::GetShaderLibrary()->TryGet("AuroraPBRStatic"));
			s_Props.FlipOnLoad = true;
			s_Props.AnisotropicFiltering = 16.0f;
			s_Texture = Texture2D::Create("Resources/textures/Qiyana2.png", s_Props);

			//const auto& [radiance, irradiance] = Renderer::CreateEnvironmentMap("Resources/environment/dikhololo_night_4k.hdr");
			//m_Environment = Environment::Create(radiance, irradiance);

			//s_ModelUniBuffer = UniformBuffer::Create(sizeof(glm::mat4) + sizeof(int), 1);
			// Model still doesnt work since models are now loaded differently and need to adapt to that
			//s_ModelShader = Renderer::GetShaderLibrary()->Get("Model"); // TODO: Temp...
			s_Created = true;
		}
	}

	Scene::~Scene()
	{
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

	void Scene::OnUpdateEditor(TimeStep ts, const EditorCamera& camera, glm::vec4 puh, glm::mat4 trans) // TODO: TEMPORARY!!!!!!!!!
	{
		Renderer3D::BeginScene(camera);

		// TODO: Rework all this shit for skylights
		auto skyLightView = m_Registry.view<TransformComponent, SkyLightComponent>();
		if (skyLightView.empty())
			Renderer3D::DrawSkyBox(Renderer::GetBlackCubeTexture(), 0.0f, 0.0f);
		for (auto entity : skyLightView)
		{
			auto [tranform, skyLight] = skyLightView.get<TransformComponent, SkyLightComponent>(entity);
			m_Environment = skyLight.SceneEnvironment;
			m_EnvironmentIntensity = skyLight.Intensity;
			m_EnvironmentLOD = skyLight.Level;
			if (m_Environment)
			{
				Renderer3D::DrawSkyBox(m_Environment->RadianceMap, m_EnvironmentLOD, m_EnvironmentIntensity);
			}
		}


		auto spriteRendererView = m_Registry.view<TransformComponent, SpriteRendererComponent>();
		for (auto entity : spriteRendererView)
		{
			auto [transform, sprite] = spriteRendererView.get<TransformComponent, SpriteRendererComponent>(entity);

			Renderer3D::DrawRotatedQuad(transform.Translation, transform.Rotation, transform.Scale, sprite.Color, (int)entity);
		}

		// entities with camera components are rendered as white planes for now!
		auto cameraView = m_Registry.view<TransformComponent, CameraComponent>();
		for (auto entity : cameraView)
		{
			auto [transform, camera] = cameraView.get<TransformComponent, CameraComponent>(entity);

			// TODO: Fix the way the camera icon is displayed
			Renderer3D::DrawRotatedQuad(transform.Translation, transform.Rotation, { transform.Scale.x, transform.Scale.y, 0.0f },
				EditorResources::CameraIcon, 1.0f, glm::vec4(1.0f), (int)entity);
		}

		auto ModelView = m_Registry.view<TransformComponent, ModelComponent>(); // TODO: Rework...!!!
		for (auto entity : ModelView)
		{
			auto[transform, modelComp] = ModelView.get<TransformComponent, ModelComponent>(entity);

			auto model = modelComp.model;

			auto rotation = glm::toMat4(glm::quat(transform.Rotation));
			auto trans = glm::translate(glm::mat4(1.0f), transform.Translation) * rotation * glm::scale(glm::mat4(1.0f), transform.Scale);

			s_ModelShader->Bind();

			s_ModelUniBuffer->SetData(glm::value_ptr(trans), sizeof(glm::mat4));
			s_ModelUniBuffer->SetData(&entity, sizeof(int), sizeof(glm::mat4));
			model.Draw(*(s_ModelShader.raw()));
		}

		Renderer3D::EndScene();

		// Rendered last so that blending can work fine with all the other batched quads
		s_Mat->Set("u_AlbedoTexture", s_Texture);
		Renderer3D::DrawMaterial(s_Mat, trans, puh); // TODO: TEMPORARY!!!!!!!!!
	}

	void Scene::OnUpdateRuntime(TimeStep ts)
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

		SceneCamera* mainCamera = nullptr;
		glm::mat4 mainTransform;
		{
			auto view = m_Registry.view<TransformComponent, CameraComponent>();
			for (auto entity : view)
			{
				auto[transform, camera] = view.get<TransformComponent, CameraComponent>(entity);

				if (camera.Primary)
				{
					mainCamera = &camera.Camera;
					mainTransform = transform.GetTransform();
				}
			}
		}

		if (mainCamera)
		{
			Renderer3D::BeginScene(*mainCamera, mainTransform);

			auto view = m_Registry.view<TransformComponent, SpriteRendererComponent>();
			for (auto entity : view)
			{
				auto[transform, sprite] = view.get<TransformComponent, SpriteRendererComponent>(entity);

				Renderer3D::DrawRotatedQuad(transform.Translation, transform.Rotation, transform.Scale, sprite.Color, (int)entity);
			}

			Renderer3D::EndScene();
		}
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		if (m_ViewportWidth == width && m_ViewportHeight == height)
			return;

		m_ViewportWidth = width;
		m_ViewportHeight = height;

		// Resize the non-fixed aspect ratio cameras...
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
				return Entity{ entity, this };
		}

		return {};
	}

}