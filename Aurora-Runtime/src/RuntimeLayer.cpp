#include "RuntimeLayer.h"

namespace Aurora {

	// TODO: Switch to projectPath once projects are a thing
	RuntimeLayer::RuntimeLayer(std::string_view scenePath)
		: m_EditorCamera(45.0f, 1280.0f, 720.0f, 0.1f, 1000.0f), m_ScenePath(scenePath)
	{
	}

	RuntimeLayer::~RuntimeLayer()
	{
	}

	void RuntimeLayer::OnAttach()
	{
		// TODO: Switch to OpenProject once projects are a thing
		OpenScene(m_ScenePath);

		// TODO: Handle sceneRendereSpecification and rendering scales and quality once shadow mapping and postfx exist

		m_SceneRenderer = SceneRenderer::Create(m_RuntimeScene);
		m_SceneRenderer->GetOptions().ShowGrid = false;

		m_Renderer2D = Renderer2D::Create();
		m_Renderer2D->SetLineWidth(2.0f);

		// Setup final image pass
		FramebufferSpecification compositeFramebuffSpec = {};
		compositeFramebuffSpec.SwapChainTarget = true;
		compositeFramebuffSpec.Width = 1280;
		compositeFramebuffSpec.Height = 720;
		compositeFramebuffSpec.DebugName = "SceneComposite";
		compositeFramebuffSpec.AttachmentsSpecification = { ImageFormat::RGBA };
		compositeFramebuffSpec.ClearColor = { 0.5f, 0.5f, 0.1f, 1.0f };

		RenderPassSpecification compositePassSpec = {};
		compositePassSpec.DebugName = "SceneCompositePass";
		compositePassSpec.TargetFramebuffer = Framebuffer::Create(compositeFramebuffSpec);
		m_FinalCompositeRenderPass = RenderPass::Create(compositePassSpec);

		Ref<Shader> textureShader = Renderer::GetShaderLibrary()->Get("TexturePass");
		m_FinalCompositeMaterial = Material::Create("SceneCompositeMat", textureShader);
		m_FinalCompositeMaterial->SetFlag(MaterialFlag::DepthTest, false); // No depth write
		m_FinalCompositeMaterial->SetFlag(MaterialFlag::TwoSided, true); // No backface culling

		// TODO: TEMPORARY...
		class CameraScript : public ScriptableEntity
		{
			virtual void OnUpdate(TimeStep ts) override
			{
				const auto& [x, y] = Input::GetMousePosition();
				const glm::vec2& mouse{ x, y };
				const glm::vec2 delta = (mouse - m_InitialMousePosition) * 0.002f;

				auto& translation = GetComponent<TransformComponent>().Translation;
				float speed = 10.0f;

				if (Input::IsKeyPressed(AR_KEY_LEFT_SHIFT))
					speed *= 2.0f;
				else if (Input::IsKeyPressed(AR_KEY_LEFT_CONTROL))
					speed *= 0.5f;

				if (Input::IsKeyPressed(AR_KEY_W))
					translation.x += speed * ts;
				else if (Input::IsKeyPressed(AR_KEY_S))
					translation.x -= speed * ts;
				if (Input::IsKeyPressed(AR_KEY_A))
					translation.z -= speed * ts;
				else if (Input::IsKeyPressed(AR_KEY_D))
					translation.z += speed * ts;
				if (Input::IsKeyPressed(AR_KEY_Q))
					translation.y += speed * ts;
				else if (Input::IsKeyPressed(AR_KEY_E))
					translation.y -= speed * ts;

				if (Input::IsMouseButtonPressed(AR_MOUSE_BUTTON_RIGHT))
				{
					TransformComponent& transform = GetComponent<TransformComponent>();
					transform.Rotation.x += -delta.y;
					transform.Rotation.y += delta.x;
				}

				m_InitialMousePosition = mouse;
			}

		private:
			glm::vec2 m_InitialMousePosition = { 0.0f, 0.0f };
		};

		Entity cameraEntity = m_RuntimeScene->GetEntityByName("Camera");
		cameraEntity.AddComponent<NativeScriptComponent>().Bind<CameraScript>();

		OnScenePlay();
	}

	void RuntimeLayer::OnDetach()
	{
		OnSceneStop();
	}

	void RuntimeLayer::OnUpdate(TimeStep ts)
	{
		auto [width, height] = Application::GetApp().GetWindow().GetSize();
		m_SceneRenderer->SetViewportSize(width, height);
		m_RuntimeScene->OnViewportResize(width, height);
		m_EditorCamera.SetViewportSize(width, height);

		if (m_Width != width || m_Height != height)
		{
			m_Width = width;
			m_Height = height;

			// Eventhough resizing the framebuffer is useless it will help in setting the viewport to the correct size
			m_FinalCompositeRenderPass->GetSpecification().TargetFramebuffer->Resize(width, height);
		}

		if (m_ViewportPanelFocused)
			m_EditorCamera.OnUpdate(ts);

		m_RuntimeScene->OnRenderRuntime(m_SceneRenderer, ts);

		// Get the final non MSAA image!
		Ref<Texture2D> finalImage = m_SceneRenderer->GetFinalPassImage();
		if (finalImage)
		{
			m_FinalCompositeMaterial->Set("u_Texture", finalImage);

			Renderer::BeginRenderPass(m_FinalCompositeRenderPass);
			Renderer::SubmitFullScreenQuad(m_FinalCompositeMaterial);
			Renderer::EndRenderPass(m_FinalCompositeRenderPass);
		}
		else
		{
			// Clear Render pass
			Renderer::BeginRenderPass(m_FinalCompositeRenderPass);
			Renderer::EndRenderPass(m_FinalCompositeRenderPass);
		}
	}

	void RuntimeLayer::OnTick()
	{
		// Nothing to tick...
	}

	void RuntimeLayer::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<KeyPressedEvent>(AR_SET_EVENT_FN(RuntimeLayer::OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(AR_SET_EVENT_FN(RuntimeLayer::OnMouseButtonPressed));
	}

	bool RuntimeLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		// TODO: Show Debug display...
		switch (e.GetKeyCode())
		{
			case KeyCode::Escape:
				Application::GetApp().Close();
				break;
		}

		return false;
	}

	bool RuntimeLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		return false;
	}

	void RuntimeLayer::OpenScene(const std::string& filePath)
	{
		Ref<Scene> newScene = Scene::Create("New Scene");
		SceneSerializer serializer(newScene);
		serializer.DeSerializeFromText(filePath);
		m_RuntimeScene = newScene;

		std::filesystem::path path = filePath;
		UpdateWindowTitle(path.filename().string());
	}

	void RuntimeLayer::OnScenePlay()
	{
		m_RuntimeScene->OnRuntimeStart();
	}

	void RuntimeLayer::OnSceneStop()
	{
		m_RuntimeScene->OnRuntimeStop();
	}

	void RuntimeLayer::UpdateWindowTitle(const std::string& sceneName)
	{
		Application::GetApp().GetWindow().SetTitle(sceneName);
	}

}