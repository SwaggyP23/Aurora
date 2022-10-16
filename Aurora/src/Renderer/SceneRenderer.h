#pragma once

#include "Core/Base.h"
#include "Scene/Scene.h"
#include "Renderer2D.h"
#include "DebugRenderer.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Camera.h"
#include "Graphics/Material.h"
#include "Graphics/Mesh.h"

namespace Aurora {

	struct SceneRendererOptions
	{
		bool ShowGrid = true;
	};

	struct SceneRendererCamera
	{
		Camera Camera;
		glm::mat4 ViewMatrix;
		float NearClip;
		float FarClip;
		float FOV;
	};

	class SceneRenderer : public RefCountedObject
	{
	public:
		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t Meshes = 0;
			uint32_t Instances = 0;
			uint32_t SavedDraws = 0;
		};

	public:
		SceneRenderer(Ref<Scene> scene);
		virtual ~SceneRenderer();

		static Ref<SceneRenderer> Create(Ref<Scene> scene);

		void SetScene(Scene* scene);

		void SetViewportSize(uint32_t width, uint32_t height);

		void BeginScene(const SceneRendererCamera& camera, glm::vec3& albedo, glm::vec3& controls);
		void EndScene();

		void SubmitStaticMesh(Ref<StaticMesh> staticMesh, Ref<MaterialTable> matTable, const glm::mat4& transform = glm::mat4(1.0f), Ref<Material> overrideMat = nullptr);

		Ref<Renderer2D> GetRenderer2D() { return m_Renderer2D; }
		Ref<DebugRenderer> GetDebugRenderer() { return m_DebugRenderer; }

		SceneRendererOptions& GetOptions() { return m_Options; }

		uint32_t GetViewportWidth() const { return m_ViewportWidth; }
		uint32_t GetViewportHeight() const { return m_ViewportHeight; }

		const Statistics& GetStatistics() const { return m_Statistics; }

		// This will return the image that will be created after all the external compositing is done since it is added to the
		// externalCompositing framebuffer as an existing image and so it is kind of a shared attachment between two framebuffers
		Ref<Texture2D> GetFinalPassImage();
		void GetFinalPassImageData(void* outPixels);
		Ref<RenderPass> GetCompositeRenderPass() { return m_CompositeRenderPass; }
		Ref<RenderPass> GetExternalCompositeRenderPass() { return m_ExternalCompositeRenderPass; }
		Ref<RenderPass> GetGeometryRenderPass() { return m_GeometryRenderPass; }

		bool IsActive() const { return m_Active; }

		float GetLineWidth() const { return m_LineWidth; }
		void SetLineWidth(float width);

	private:
		void FlushDrawList();

		void PrePass();

		void GeometryPass();
		void CompositePass();

		void UpdateStatistics();

	private:
		Ref<Scene> m_Scene = nullptr;

		Ref<Renderer2D> m_Renderer2D = nullptr;
		Ref<DebugRenderer> m_DebugRenderer = nullptr;

		struct SceneInfo
		{
			SceneRendererCamera SceneCamera;

			// Resources
			Ref<Environment> SceneEnvironment;
			float SkyboxLod = 0.0f;
			float SceneEnvironmentIntensity;

			LightEnvironment SceneLightEnvironment;
		} m_SceneData;

		struct UBScreenData
		{
			glm::vec2 FullResolution;
			glm::vec2 InvFullResolution;
			glm::vec2 HalfResolution;
			glm::vec2 InvHalfResolution;
		} m_ScreenData;

		Ref<UniformBuffer> m_ScreenDataUB;

		struct UBCameraData
		{
			glm::mat4 ViewProjectionMatrix;
			glm::mat4 InverseViewProjectionMatrix;
			glm::mat4 ProjectionMatrix;
			glm::mat4 InverseProjectionMatrix;
			glm::mat4 ViewMatrix;
			glm::mat4 InverseViewMatrix;
			float NearClip;
			float FarClip;
			float FOV; // Radians
			float Padding0;
		} m_CameraData;

		Ref<UniformBuffer> m_CameraDataUB;

		struct DirLight
		{
			glm::vec3 Direction;
			glm::vec3 Radiance;
			float Intensity;
		};

		struct UBScene
		{
			DirLight Lights;
			glm::vec3 CameraPosition;
			float EnvironmentMapIntensity = 1.0f;
		} m_SceneDirLightData;
		
		Ref<UniformBuffer> m_SceneDirLightDataUB;

		Ref<RenderPass> m_GeometryRenderPass;
		Ref<RenderPass> m_CompositeRenderPass;
		Ref<RenderPass> m_ExternalCompositeRenderPass;

		Ref<Material> m_SkyboxMaterial;
		Ref<Material> m_SceneCompositeMaterial;

		struct DrawCommand
		{
			Ref<StaticMesh> StaticMesh;
			Ref<MaterialTable> MaterialTable;
			glm::mat4 Transform;
			Ref<Material> OverrideMaterial;

			uint32_t InstanceCount = 0;
			uint32_t InstanceOffset = 0;
		};
		std::vector<DrawCommand> m_StaticMeshDrawList;

		Ref<Shader> m_GridShader;
		Ref<Material> m_GridMaterial;
		
		SceneRendererOptions m_Options;
		Statistics m_Statistics;

		uint32_t m_ViewportWidth = 0;
		float m_InvViewportWidth = 0.0f;
		uint32_t m_ViewportHeight = 0;
		float  m_InvViewportHeight = 0.0f;

		bool m_Active = false;
		bool m_NeedsResize = false;

		float m_LineWidth = 2.0f;

	};

}