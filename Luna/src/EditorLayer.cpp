#include "EditorLayer.h"

#include <glm/gtc/constants.hpp>
#include <glm/gtx/compatibility.hpp>

namespace Aurora {

	EditorLayer::EditorLayer()
		: Layer("BatchRenderer"),
		m_Camera(Aurora::CreateRef<Aurora::EditorCamera>(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f)),
		m_OrthoCamera(Aurora::CreateRef<Aurora::OrthoGraphicCamera>(16.0f / 9.0f, -100.0f, 100.0f))
	{
	}

	void EditorLayer::onAttach()
	{
		PROFILE_FUNCTION();

		m_ContainerTexture = Aurora::Texture::Create("assets/textures/container2.png");
		m_ContainerTexture->flipTextureVertically(true);
		m_ContainerTexture->setTextureWrapping(GL_REPEAT);
		m_ContainerTexture->setTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
		m_ContainerTexture->loadTextureData();

		m_GroundTexture = Aurora::Texture::Create("assets/textures/ice.png");
		m_GroundTexture->flipTextureVertically(true);
		m_GroundTexture->setTextureWrapping(GL_REPEAT);
		m_GroundTexture->setTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
		m_GroundTexture->loadTextureData();
	}
	void EditorLayer::onDetach()
	{
		PROFILE_FUNCTION();
	}

	void EditorLayer::onUpdate(Aurora::TimeStep ts)
	{
		PROFILE_FUNCTION();

		{
			PROFILE_SCOPE("Clear Colors");
			Aurora::RenderCommand::setClearColor(m_Color);
			Aurora::RenderCommand::Clear();
		}

		Aurora::Renderer3D::ResetStats();

		if (m_Perspective)
			Aurora::Renderer3D::BeginScene(m_Camera);
		else
			Aurora::Renderer3D::BeginScene(m_OrthoCamera);

		{
			PROFILE_SCOPE("Rendering");
			Aurora::Renderer3D::DrawQuad({ 1.2f, 3.0f, 2.0f }, { 0.2f, 0.2f, 0.2f }, { 1.0f, 1.0f, 1.0f, 1.0f }, 1);
			Aurora::Renderer3D::DrawQuad({ 0.0f, -7.0f, 0.0f }, { 30.0f, 2.0f, 30.0f }, m_GroundTexture, 20.0f);
			Aurora::Renderer3D::DrawQuad({ 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f }, m_ContainerTexture);
			Aurora::Renderer3D::DrawQuad({ -2.0f, 2.0f, 1.0f }, { 1.0f, 1.0f, 1.0f }, m_ContainerTexture);
			Aurora::Renderer3D::DrawQuad({ 1.0f,-4.0f, 2.0f }, { 1.0f, 1.0f, 1.0f }, m_ContainerTexture);
			Aurora::Renderer3D::DrawQuad({ 1.0f, 3.0f,-2.0f }, { 1.0f, 1.0f, 1.0f }, m_ContainerTexture);
			Aurora::Renderer3D::DrawQuad({ 5.0f, 2.0f,-1.0f }, { 1.0f, 1.0f, 1.0f }, m_ContainerTexture);
			Aurora::Renderer3D::DrawRotatedQuad(m_Transalations, m_Rotations, m_Scales, m_UniColor);
			Aurora::Renderer3D::DrawQuad({ 0.0f, 0.0f, -10.1f }, { 10.0f, 10.0f, 0.0f }, m_GroundTexture, 30.0f);

			for (float y = -5.0f; y < 5.0f; y += 0.5f)
			{
				for (float x = -5.0f; x < 5.0f; x += 0.5f)
				{
					glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f };
					Aurora::Renderer3D::DrawQuad({ x, y, -10.0f }, { 0.45f, 0.45f, 0.0f }, color);
				}
			}

			static float rotation;
			rotation += ts * 50.0f;
			Aurora::Renderer3D::DrawRotatedQuad({ -5.5f, -1.5f, -6.0f }, { m_Rotations.x, m_Rotations.y, rotation }, { 3.0f, 3.0f, 3.0f }, { 0, 247.0f / 255.0f, 168.0f / 255.0f, 0.7f });
			//Aurora::Renderer3D::DrawRotatedQuad(m_Transalations, m_Rotations, m_Scales, m_CheckerTexture, 20.0f, m_UniColor);
			//Aurora::Renderer3D::DrawQuad({ -1.0f,  0.0f, -8.0f }, { 0.8f, 0.8f, 1.0f }, { 0.8f, 0.2f, 0.3f, 1.0f });
			//Aurora::Renderer3D::DrawQuad({  1.5f, -0.5f, -8.0f }, { 0.5f, 0.75f, 1.0f }, { 0.2f, 0.3f, 0.8f, 1.0f });
			//Aurora::Renderer3D::DrawQuad({ 1.0f,  -1.0f,  0.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, 1);
		}

		Aurora::Renderer3D::EndScene();

		if (m_Perspective)
			m_Camera->OnUpdate(ts);
		else
			m_OrthoCamera->OnUpdate(ts);
	}

	void EditorLayer::onEvent(Aurora::Event& e)
	{
		if (m_Perspective)
			m_Camera->OnEvent(e);
		else
			m_OrthoCamera->OnEvent(e);
	}

	void EditorLayer::onImGuiRender()
	{
		PROFILE_FUNCTION();

		Aurora::Application& app = Aurora::Application::getApp(); // Currently imgui does nothing since its input is not passed on

		ImGui::Begin("Editing Panel");
		if (ImGui::CollapsingHeader("Cube")) {
			ImGui::ColorEdit3("Uniform Color", (float*)&m_UniColor);
			ImGui::SliderFloat3("Cube Translation", (float*)&m_Transalations, -5.0f, 5.0f);
			ImGui::SliderFloat3("Cube Rotations", (float*)&m_Rotations, 0.0f, 360.0f);
			ImGui::SliderFloat3("Cube Scale", (float*)&m_Scales, 0.0f, 3.0f);
		}

		ImGui::Separator();

		if (ImGui::CollapsingHeader("Right Wall")) {
			ImGui::SliderFloat3("Wall Translation", (float*)&m_WallTransalations, -5.0f, 5.0f);
			ImGui::SliderFloat3("Wall Rotations", (float*)&m_WallRotations, 0.0f, 360.0f);
			ImGui::SliderFloat3("Wall Scale", (float*)&m_WallScales, 0.0f, 3.0f);
		}

		ImGui::Separator();

		ImGui::ColorEdit3("Clear Color", (float*)&m_Color);

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
		ImGui::Text("Framerate: %.f", ImGui::GetIO().Framerate);
		ImGui::Text("Draw Calls: %d", Aurora::Renderer3D::GetStats().DrawCalls);
		ImGui::Text("Quad Count: %d", Aurora::Renderer3D::GetStats().QuadCount);
		ImGui::Text("Vertex Count: %d", Aurora::Renderer3D::GetStats().GetTotalVertexCount());
		ImGui::Text("Index Count: %d", Aurora::Renderer3D::GetStats().GetTotalIndexCount());
		ImGui::Text("Vertex Buffer Memory: %.3f MegaBytes", Aurora::Renderer3D::GetStats().GetTotalVertexBufferMemory() / (1024.0f * 1024.0f));
		ImGui::Checkbox("V Sync ", &(app.getVSync()));
		ImGui::Text("Peak FPS: %.f", m_Peak);

		ImGui::End();
	}

}