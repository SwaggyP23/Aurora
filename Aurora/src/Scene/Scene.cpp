#include "Aurorapch.h"
#include "Scene.h"
#include "Entity.h"

#include "Graphics/Model.h"
#include "Graphics/UniformBuffer.h" // TODO: Temp...!
#include "Components.h"
#include "ScriptableEntity.h"
#include "Renderer/Renderer3D.h"

#include <glm/gtc/type_ptr.hpp>

namespace Aurora {

	static Ref<UniformBuffer> s_ModelUniBuffer;

	Ref<Scene> Scene::Create(const std::string& debugName)
	{
		return CreateRef<Scene>(debugName);
	}

	Scene::Scene(const std::string& debugName)
		: m_Name(debugName)
	{
		m_ModelShader = Shader::Create("Resources/shaders/model.glsl"); // TODO: Temp...
		s_ModelUniBuffer = UniformBuffer::Create(sizeof(glm::mat4) + sizeof(int), 1);
		m_EnvironmentMap = CubeTexture::Create("Resources/environment/skybox");
	}

	Scene::~Scene()
	{
	}

	Entity Scene::CreateEntity(const char* name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name == "" ? "AuroraDefault" : name;

		return entity;
	}

	Entity Scene::CopyEntity(Entity entity)
	{
		AR_CORE_ASSERT(false);

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

	void Scene::OnUpdateEditor(TimeStep ts, EditorCamera& camera)
	{
		Renderer3D::BeginScene(camera);

		Renderer3D::DrawSkyBox(m_EnvironmentMap);

		auto view = m_Registry.view<TransformComponent, SpriteRendererComponent>();
		for (auto entity : view)
		{
			auto [transform, sprite] = view.get<TransformComponent, SpriteRendererComponent>(entity);

			Renderer3D::DrawRotatedQuad(transform.Translation, transform.Rotation, transform.Scale, sprite.Color, 0, (int)entity);
		}

		// entities with camera components are rendered as white planes for now!
		auto cameraView = m_Registry.view<TransformComponent, CameraComponent>();
		for (auto entity : cameraView)
		{
			auto [transform, camera] = cameraView.get<TransformComponent, CameraComponent>(entity);

			Renderer3D::DrawRotatedQuad(transform.Translation, transform.Rotation, { transform.Scale.x, transform.Scale.y, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, 0, (int)entity);
		}

		auto ModelView = m_Registry.view<TransformComponent, ModelComponent>(); // TODO: Rework...!!!
		for (auto entity : ModelView)
		{
			auto[transform, modelComp] = ModelView.get<TransformComponent, ModelComponent>(entity);

			auto model = modelComp.model;

			auto rotation = glm::toMat4(glm::quat(transform.Rotation));
			auto trans = glm::translate(glm::mat4(1.0f), transform.Translation) * rotation * glm::scale(glm::mat4(1.0f), transform.Scale);

			m_ModelShader->Bind();

			s_ModelUniBuffer->SetData(glm::value_ptr(trans), sizeof(glm::mat4));
			s_ModelUniBuffer->SetData(&entity, sizeof(int), sizeof(glm::mat4));
			model.Draw(*(m_ModelShader.raw()));
		}

		Renderer3D::EndScene();
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

		Camera* mainCamera = nullptr;
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
			Renderer3D::BeginScene(mainCamera->GetProjection(), mainTransform);

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

	// TODO: ReWrite this system.
	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(sizeof(T) == 0);
	}

	template<>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		if(m_ViewportWidth > 0 &&  m_ViewportHeight > 0)
			component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}

	template<>
	void Scene::OnComponentAdded<ModelComponent>(Entity entity, ModelComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{
	}

}