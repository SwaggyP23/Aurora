#include "EditorLayer.h"

#include <glm/gtc/constants.hpp>
#include <glm/gtx/compatibility.hpp>

namespace Aurora {

	static void ShowTimers()
	{
		auto& mp = PerformanceTimer::GetTimeMap();

		std::vector<std::pair<std::string, float>> timerValues;
		timerValues.reserve(mp.size());
		for (auto& [name, val] : mp)
			timerValues.emplace_back(name, val);

		std::sort(timerValues.begin(), timerValues.end(), [](const std::pair<std::string, float>& a, const std::pair<std::string, float>& b) -> bool
			{
				return a.second > b.second;
			});

		for (auto& it : timerValues)
		{
			ImGui::Text("%.4f, %s", it.second, it.first.c_str());
		}
	}

	EditorLayer::EditorLayer()
		: Layer("BatchRenderer"),
		m_Camera(CreateRef<EditorCamera>(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f)),
		m_OrthoCamera(CreateRef<OrthoGraphicCamera>(16.0f / 9.0f, -100.0f, 100.0f))
	{
	}

	void EditorLayer::OnAttach()
	{
		AR_PROFILE_FUNCTION();

		m_ContainerTexture = Texture::Create("resources/textures/Qiyana.jpg");
		m_ContainerTexture->flipTextureVertically(true);
		m_ContainerTexture->setTextureWrapping(TextureProperties::Repeat);
		m_ContainerTexture->setTextureFiltering(TextureProperties::MipMap_LinearLinear, TextureProperties::Linear);
		m_ContainerTexture->loadTextureData();

		m_GroundTexture = Texture::Create("resources/textures/ice.png");
		m_GroundTexture->flipTextureVertically(true);
		m_GroundTexture->setTextureWrapping(TextureProperties::Repeat);
		m_GroundTexture->setTextureFiltering(TextureProperties::MipMap_LinearLinear, TextureProperties::Linear);
		m_GroundTexture->loadTextureData();

		m_QiyanaTexture = Texture::Create("resources/textures/checkerboard.png");
		m_QiyanaTexture->flipTextureVertically(true);
		m_QiyanaTexture->setTextureWrapping(TextureProperties::Repeat);
		m_QiyanaTexture->setTextureFiltering(TextureProperties::MipMap_LinearLinear, TextureProperties::Linear);
		m_QiyanaTexture->loadTextureData();

		m_CheckerTexture = Texture::Create("resources/textures/checkerboard2.png");
		m_CheckerTexture->flipTextureVertically(true);
		m_CheckerTexture->setTextureWrapping(TextureProperties::Repeat);
		m_CheckerTexture->setTextureFiltering(TextureProperties::MipMap_LinearLinear, TextureProperties::Linear);
		m_CheckerTexture->loadTextureData();

		FramebufferSpecification fbSpec;
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		m_Framebuffer = Framebuffer::Create(fbSpec);

		m_ActiveScene = Scene::Create();

		auto square = m_ActiveScene->CreateEntity("Test Square");
		square.AddComponent<SpriteRendererComponent>(glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f });

		m_SquareEntity = square;
	}

	void EditorLayer::OnDetach()
	{
		AR_PROFILE_FUNCTION();
	}

	void EditorLayer::OnUpdate(TimeStep ts)
	{
		AR_PROFILE_FUNCTION();
		AR_PERF_TIMER("EditorLayer::OnUpdate");

		if (m_ViewPortFocused) {
			if (m_Perspective)
				m_Camera->OnUpdate(ts);
			else
				m_OrthoCamera->OnUpdate(ts);
		}

		Renderer3D::ResetStats();

		m_Framebuffer->bind();
		RenderCommand::setClearColor(m_Color);
		RenderCommand::Clear();


		if (m_Perspective)
			Renderer3D::BeginScene(m_Camera);
		else
			Renderer3D::BeginScene(m_OrthoCamera);

		m_ActiveScene->onUpdate(ts);

		Renderer3D::EndScene();

		m_Framebuffer->unBind();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		if (m_Perspective)
			m_Camera->OnEvent(e);
		else
			m_OrthoCamera->OnEvent(e);
	}

	void EditorLayer::OnImGuiRender()
	{
		AR_PROFILE_FUNCTION();

		Application& app = Application::getApp(); // Currently imgui does nothing since its input is not passed on

		static bool dockSpaceOpen = true;
		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		if (!opt_padding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockSpaceOpen, window_flags);
		if (!opt_padding)
			ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Options"))
			{
				// Disabling fullscreen would allow the window to be moved to the front of other windows,
				// which we can't undo at the moment without finer window depth/z control.

				if (ImGui::MenuItem("Performance", NULL, m_Performance)) m_Performance = !m_Performance;
				if (ImGui::MenuItem("Exit")) Application::getApp().Close();

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::Begin("Editing Panel");

		ImGui::ColorEdit3("Clear Color", (float*)&m_Color);

		if (m_SquareEntity)
		{
			ImGui::Separator();
			auto& name = m_SquareEntity.GetComponent<TagComponent>().Tag;
			ImGui::Text("%s", name.c_str());

			auto& squareColor = m_SquareEntity.GetComponent<SpriteRendererComponent>().Color;
			ImGui::ColorEdit3("Cube Color", (float*)&squareColor);
		}

		ImGui::Separator();
		//ImGui::ShowDemoWindow(); // For reference

		ImGui::Checkbox("Camera Type:", &m_Perspective);
		ImGui::SameLine();
		if (m_Perspective)
			ImGui::Text("Perspective Camera!");
		else
			ImGui::Text("OrthoGraphic Camera!");


		float peak = std::max(m_Peak, ImGui::GetIO().Framerate);
		m_Peak = peak;
		ImGui::Separator();
		ImGui::Text("Renderer Stats:");
		ImGui::Text("Draw Calls: %d", Renderer3D::GetStats().DrawCalls);
		ImGui::Text("Quad Count: %d", Renderer3D::GetStats().QuadCount);
		ImGui::Text("Vertex Count: %d", Renderer3D::GetStats().GetTotalVertexCount());
		ImGui::Text("Index Count: %d", Renderer3D::GetStats().GetTotalIndexCount());
		ImGui::Text("Vertex Buffer Memory: %.3f MegaBytes", Renderer3D::GetStats().GetTotalVertexBufferMemory() / (1024.0f * 1024.0f));
		ImGui::Checkbox("V Sync ", &(app.getVSync()));

		ImGui::End();

		if (m_Performance)
		{
			ImGui::Begin("Performance");
			ImGui::Text("Framerate: %.f", ImGui::GetIO().Framerate);
			ImGui::Text("Peak FPS: %.f", m_Peak);
			
			if (ImGui::CollapsingHeader("CPU Timers (Milliseconds)"))
				ShowTimers();

			ImGui::End();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport");

		m_ViewPortFocused = ImGui::IsWindowFocused();
		m_ViewPortHovered = ImGui::IsWindowHovered();

		// If viewport is not focused OR is not hovered -> Block events
		// Which means if we lost focus however we are still hovered, that is not acceptable -> Block events
		Application::getApp().getImGuiLayer()->SetBlockEvents(!m_ViewPortFocused || !m_ViewPortHovered);

		ImVec2 viewPortPanelSize = ImGui::GetContentRegionAvail();
		if (m_ViewportSize != *((glm::vec2*)&viewPortPanelSize))
		{
			m_ViewportSize = { viewPortPanelSize.x, viewPortPanelSize.y };
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		uint32_t textureID = m_Framebuffer->GetColorAttachmentID();
		ImGui::Image((void*)textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::End();
	}

}