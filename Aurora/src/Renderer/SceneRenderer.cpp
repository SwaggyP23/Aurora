#include "Aurorapch.h"
#include "SceneRenderer.h"

#include "Renderer.h"

namespace Aurora {

	Ref<SceneRenderer> SceneRenderer::Create(Ref<Scene> scene)
	{
		return CreateRef<SceneRenderer>(scene);
	}

	SceneRenderer::SceneRenderer(Ref<Scene> scene)
		: m_Scene(scene)
	{
		AR_SCOPED_TIMER("SceneRenderer::SceneRenderer");

		// UniformBuffers
		{
			m_CameraDataUB = UniformBuffer::Create(sizeof(UBCameraData), 0); // Binding Point 0
			m_ScreenDataUB = UniformBuffer::Create(sizeof(UBScreenData), 1); // Binding Point 1
		}

		// Framebuffers/RenderPasses TODO: When we have HDR, Framebuffers should be FLOATING point framebuffers...
		{
			FramebufferSpecification geoFBOSpec = {};
			geoFBOSpec.DebugName = "GeometryFBO";
			geoFBOSpec.AttachmentsSpecification = { ImageFormat::RGBA, ImageFormat::DEPTH32F };
			geoFBOSpec.Width = 1280;
			geoFBOSpec.Height = 720;
			geoFBOSpec.Samples = 1;
			geoFBOSpec.ClearColor = { 0.5f, 0.1f, 0.1f, 1.0f };

			RenderPassSpecification geometryPassSpec = {};
			geometryPassSpec.DebugName = "GeometryPass";
			geometryPassSpec.TargetFramebuffer = Framebuffer::Create(geoFBOSpec);
			m_GeometryRenderPass = RenderPass::Create(geometryPassSpec);

			FramebufferSpecification compositeFBOSpec = {};
			compositeFBOSpec.DebugName = "CompositeFBO";
			compositeFBOSpec.AttachmentsSpecification = { ImageFormat::RGBA };
			compositeFBOSpec.Width = 1280;
			compositeFBOSpec.Height = 720;
			compositeFBOSpec.ClearColor = { 0.5f, 0.1f, 0.1f, 1.0f };

			RenderPassSpecification compositePassSpec = {};
			compositePassSpec.DebugName = "CompositePass";
			compositePassSpec.TargetFramebuffer = Framebuffer::Create(compositeFBOSpec);
			m_CompositeRenderPass = RenderPass::Create(compositePassSpec);

			FramebufferSpecification externalCompositeFBO = {};
			externalCompositeFBO.DebugName = "ExternalCompositeFBO";
			externalCompositeFBO.AttachmentsSpecification = { ImageFormat::RGBA, ImageFormat::DEPTH32F };
			externalCompositeFBO.Width = 1280;
			externalCompositeFBO.Height = 720;
			externalCompositeFBO.ClearColor = { 0.0f, 0.1f, 0.5f, 1.0f };
			externalCompositeFBO.ClearOnBind = false;
			externalCompositeFBO.ExistingImages[0] = m_CompositeRenderPass->GetSpecification().TargetFramebuffer->GetColorAttachment(0);
			externalCompositeFBO.ExistingImages[1] = m_GeometryRenderPass->GetSpecification().TargetFramebuffer->GetDepthAttachment();

			RenderPassSpecification externalCompositingPass = {};
			externalCompositingPass.DebugName = "ExternalCompositePass";
			externalCompositingPass.TargetFramebuffer = Framebuffer::Create(externalCompositeFBO);
			m_ExternalCompositeRenderPass = RenderPass::Create(externalCompositingPass);
		}

		// Set up Skybox material
		{
			m_SkyboxMaterial = Material::Create("Skybox", Renderer::GetShaderLibrary()->Get("Skybox"));
			m_SkyboxMaterial->SetFlag(MaterialFlag::DepthTest, false);
		}

		// Set up Grid material
		{
			m_GridShader = Renderer::GetShaderLibrary()->Get("Grid");
			m_GridMaterial = Material::Create("Grid", m_GridShader);
			constexpr float gridSize = 0.025f;
			constexpr float gridScale = 50.025f;
			m_GridMaterial->Set("u_Settings.Scale", gridScale);
			m_GridMaterial->Set("u_Settings.Size", gridSize);
			m_GridMaterial->SetFlag(MaterialFlag::TwoSided, true);
		}

		m_Renderer2D = Renderer2D::Create();
		m_Renderer2D->SetLineWidth(m_LineWidth);
	}

	SceneRenderer::~SceneRenderer()
	{
	}

	void SceneRenderer::SetScene(Scene* scene)
	{
		AR_CORE_ASSERT(!m_Active, "Cant change scene when renderer is active!");
		m_Scene = scene;
	}

	void SceneRenderer::SetViewportSize(uint32_t width, uint32_t height)
	{
		if (m_ViewportWidth != width && m_ViewportHeight != height)
		{
			m_NeedsResize = true;

			m_ViewportWidth = width;
			m_InvViewportWidth = 1.0f / width;
			m_ViewportHeight = height;
			m_InvViewportHeight = 1.0f / height;
		}
	}

	void SceneRenderer::BeginScene(const SceneRendererCamera& camera)
	{
		AR_PROFILE_FUNCTION();

		AR_CORE_ASSERT(m_Scene);
		AR_CORE_ASSERT(!m_Active, "Cant begin scene if rendering is already active");
		m_Active = true;

		m_SceneData.SceneCamera = camera;
		m_SceneData.SceneEnvironment = m_Scene->m_Environment;
		m_SceneData.SceneEnvironmentIntensity = m_Scene->m_EnvironmentIntensity;
		m_SceneData.SkyboxLod = m_Scene->m_EnvironmentLOD;

		if (m_NeedsResize)
		{
			m_NeedsResize = false;

			m_ScreenData.FullResolution = { m_ViewportWidth, m_ViewportHeight };
			m_ScreenData.InvFullResolution = { m_InvViewportWidth, m_InvViewportHeight };
			m_ScreenData.HalfResolution = glm::ivec2(m_ScreenData.FullResolution) / 2;
			m_ScreenData.InvHalfResolution = 1.0f / m_ScreenData.HalfResolution;

			m_GeometryRenderPass->GetSpecification().TargetFramebuffer->Resize(m_ViewportWidth, m_ViewportHeight);
			m_CompositeRenderPass->GetSpecification().TargetFramebuffer->Resize(m_ViewportWidth, m_ViewportHeight);
			m_ExternalCompositeRenderPass->GetSpecification().TargetFramebuffer->Resize(m_ViewportWidth, m_ViewportHeight);

			Renderer::OnWindowResize(m_ViewportWidth, m_ViewportHeight);
		}

		// Update Uniform Buffers...
		// Camera
		{
			UBCameraData& cameraData = m_CameraData;
			glm::mat4 viewProj = std::move(m_SceneData.SceneCamera.Camera.GetProjection() * m_SceneData.SceneCamera.ViewMatrix);
			glm::mat4 viewInverse = std::move(glm::inverse(m_SceneData.SceneCamera.ViewMatrix));
			glm::mat4 projInverse = std::move(glm::inverse(m_SceneData.SceneCamera.Camera.GetProjection()));
			glm::vec3 cameraPos = viewInverse[3];

			cameraData.ViewProjectionMatrix = std::move(viewProj);
			cameraData.ProjectionMatrix = m_SceneData.SceneCamera.Camera.GetProjection();
			cameraData.InverseProjectionMatrix = std::move(projInverse);
			cameraData.ViewMatrix = std::move(m_SceneData.SceneCamera.ViewMatrix);
			cameraData.InverseViewMatrix = std::move(viewInverse);
			cameraData.InverseViewProjectionMatrix = std::move(cameraData.InverseViewMatrix * cameraData.InverseProjectionMatrix);
			cameraData.NearClip = m_SceneData.SceneCamera.NearClip;
			cameraData.FarClip = m_SceneData.SceneCamera.FarClip;
			cameraData.FOV = m_SceneData.SceneCamera.FOV;

			// Binding = 0
			m_CameraDataUB->SetData(&cameraData, sizeof(cameraData));
		}

		// Screen
		{
			UBScreenData& screenData = m_ScreenData;

			// Binding = 1
			m_ScreenDataUB->SetData(&screenData, sizeof(screenData));
		}

		Renderer::SetSceneEnvironment(this, m_SceneData.SceneEnvironment);
	}

	void SceneRenderer::EndScene()
	{
		FlushDrawList();

		m_Active = false;
	}

	void SceneRenderer::SubmitStaticMesh(Ref<StaticMesh> staticMesh, Ref<MaterialTable> matTable, const glm::mat4& transform, Ref<Material> overrideMat)
	{
		m_StaticMeshDrawList.push_back({ staticMesh, matTable, transform, overrideMat });
	}

	void SceneRenderer::SetLineWidth(float width)
	{
		m_LineWidth = width;
	}

	void SceneRenderer::GetFinalPassImageData(void* outPixels)
	{
		return m_CompositeRenderPass->GetSpecification().TargetFramebuffer->GetColorAttachmentData(outPixels, 0);
	}

	Ref<Texture2D> SceneRenderer::GetFinalPassImage()
	{
		return m_CompositeRenderPass->GetSpecification().TargetFramebuffer->GetColorAttachment(0);
	}

	Ref<RenderPass> SceneRenderer::GetCompositeRenderPass()
	{
		return m_CompositeRenderPass;
	}

	Ref<RenderPass> SceneRenderer::GetExternalCompositeRenderPass()
	{
		return m_ExternalCompositeRenderPass;
	}

	void SceneRenderer::FlushDrawList()
	{
		// For now...
		PrePass();

		GeometryPass();
		CompositePass();

		// Reset everything for the next frame after all the passes are done
		UpdateStatistics();
		m_StaticMeshDrawList.clear();
		m_SceneData = {};
	}

	void SceneRenderer::PrePass()
	{
		// Clear default framebuffer
		RenderCommand::SetClearColor(glm::vec4{ 0.1f, 0.1f, 0.1f, 1.0f });
		RenderCommand::Clear();
	}

	void SceneRenderer::GeometryPass()
	{
		Renderer::BeginRenderPass(m_GeometryRenderPass);

		// Skybox
		m_SkyboxMaterial->Set("u_Uniforms.TextureLOD", m_SceneData.SkyboxLod);
		m_SkyboxMaterial->Set("u_Uniforms.Intensity", m_SceneData.SceneEnvironmentIntensity);

		const Ref<CubeTexture>& radianceMap = m_SceneData.SceneEnvironment ? m_SceneData.SceneEnvironment->RadianceMap : Renderer::GetBlackCubeTexture();
		m_SkyboxMaterial->Set("u_RadianceMap", radianceMap);
		Renderer::SubmitFullScreenQuad(m_SkyboxMaterial);

		// Render Entities...
		for (auto& dc : m_StaticMeshDrawList)
		{
			Renderer::RenderStaticMesh(dc.StaticMesh, dc.MaterialTable ? dc.MaterialTable : dc.StaticMesh->GetMaterials(), dc.Transform);
		}

		if (GetOptions().ShowGrid)
		{
			const glm::mat4 transform = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f))
				* glm::scale(glm::mat4(1.0f), glm::vec3(8.0f));
			Renderer::RenderQuad(m_GridMaterial, transform);
		}

		Renderer::EndRenderPass(m_GeometryRenderPass);
	}

	void SceneRenderer::CompositePass()
	{
		Renderer::BeginRenderPass(m_CompositeRenderPass);

		const Ref<Framebuffer>& src = m_GeometryRenderPass->GetSpecification().TargetFramebuffer;
		const Ref<Framebuffer>& dst = m_CompositeRenderPass->GetSpecification().TargetFramebuffer;

		// Blitting both color attachments
		{
			Framebuffer::Blit(src->GetFramebufferID(),
				dst->GetFramebufferID(),
				src->GetSpecification().Width,
				src->GetSpecification().Height,
				0,
				dst->GetSpecification().Width,
				dst->GetSpecification().Height,
				0);
		}

		Renderer::EndRenderPass(m_CompositeRenderPass);
	}

	void SceneRenderer::UpdateStatistics()
	{
		m_Renderer2D->ResetStats();

		m_Statistics.DrawCalls = 0;
		m_Statistics.Instances = 0;
		m_Statistics.Meshes = 0;
		m_Statistics.SavedDraws = 0;

		for (const DrawCommand& dc : m_StaticMeshDrawList)
		{
			m_Statistics.Instances = dc.InstanceCount;
			m_Statistics.DrawCalls++; // Since each mesh is its own draw call
			m_Statistics.Meshes++;
		}

		m_Statistics.SavedDraws = m_Statistics.Instances - m_Statistics.DrawCalls;
	}

}