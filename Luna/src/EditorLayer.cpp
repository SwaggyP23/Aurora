#include "EditorLayer.h"

#include <glm/gtc/constants.hpp>
#include <glm/gtx/compatibility.hpp>

#include <iostream>

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

		m_ContainerTexture = Aurora::Texture::Create("resources/textures/container2.png");
		m_ContainerTexture->flipTextureVertically(true);
		m_ContainerTexture->setTextureWrapping(GL_REPEAT);
		m_ContainerTexture->setTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
		m_ContainerTexture->loadTextureData();

		m_GroundTexture = Aurora::Texture::Create("resources/textures/ice.png");
		m_GroundTexture->flipTextureVertically(true);
		m_GroundTexture->setTextureWrapping(GL_REPEAT);
		m_GroundTexture->setTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
		m_GroundTexture->loadTextureData();

		m_QiyanaTexture = Aurora::Texture::Create("resources/textures/checkerboard.png");
		m_QiyanaTexture->flipTextureVertically(true);
		m_QiyanaTexture->setTextureWrapping(GL_REPEAT);
		m_QiyanaTexture->setTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
		m_QiyanaTexture->loadTextureData();

		m_CheckerTexture = Aurora::Texture::Create("resources/textures/checkerboard2.png");
		m_CheckerTexture->flipTextureVertically(true);
		m_CheckerTexture->setTextureWrapping(GL_REPEAT);
		m_CheckerTexture->setTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
		m_CheckerTexture->loadTextureData();
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

			Aurora::Renderer3D::DrawQuad({ 0.0f, -7.0f, 0.0f }, { 30.0f, 2.0f, 30.0f }, {1.0f, 1.0f, 1.0f, 1.0f});
			Aurora::Renderer3D::DrawQuad({ 0.0f, -0.5f,-16.0f }, { 30.0f, 15.0f, 2.0f }, {1.0f, 1.0f, 1.0f, 1.0f});
			Aurora::Renderer3D::DrawQuad({-16.0f, -0.5f, -1.0f }, { 2.0f, 15.0f, 32.0f }, {1.0f, 1.0f, 1.0f, 1.0f});

			Aurora::Renderer3D::DrawQuad({ 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f }, m_QiyanaTexture);
			Aurora::Renderer3D::DrawQuad({ -2.0f, 2.0f, 1.0f }, { 1.0f, 1.0f, 1.0f }, m_CheckerTexture);

			//Aurora::Renderer3D::DrawQuad({ 10.1f, 0.0f, 0.0f }, { 0.0f, 10.0f, 10.0f }, m_GroundTexture, 30.0f);
			//for (float y = -5.0f; y < 5.0f; y += 0.5f)
			//{
			//	for (float x = -5.0f; x < 5.0f; x += 0.5f)
			//	{
			//		glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f };
			//		Aurora::Renderer3D::DrawQuad({ 10.0f, y, x }, { 0.0f, 0.45f, 0.45f }, color);
			//	}
			//}

			static float rotation;
			rotation += ts * 50.0f;
			Aurora::Renderer3D::DrawRotatedQuad({ -5.5f, -1.5f, -6.0f }, { 0.0f, 0.0f, rotation }, { 3.0f, 3.0f, 3.0f }, { 0, 247.0f / 255.0f, 168.0f / 255.0f, 0.7f });
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