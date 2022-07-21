#include "TestLayer.h"

#include <glm/gtc/constants.hpp>
#include <glm/gtx/compatibility.hpp>

TestLayer::TestLayer()
	: Layer("BatchRenderer"),
	m_Camera(Aurora::EditorCamera(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f))
{
}

void TestLayer::OnAttach()
{
	AR_PROFILE_FUNCTION();

	m_ContainerTexture = Aurora::Texture::Create("resources/textures/container2.png");
	m_ContainerTexture->flipTextureVertically(true);
	m_ContainerTexture->setTextureWrapping(Aurora::TextureProperties::Repeat);
	m_ContainerTexture->setTextureFiltering(Aurora::TextureProperties::MipMap_LinearLinear, Aurora::TextureProperties::Linear);
	m_ContainerTexture->loadTextureData();

	m_GroundTexture = Aurora::Texture::Create("resources/textures/ice.png");
	m_GroundTexture->flipTextureVertically(true);
	m_GroundTexture->setTextureWrapping(Aurora::TextureProperties::Repeat);
	m_GroundTexture->setTextureFiltering(Aurora::TextureProperties::MipMap_LinearLinear, Aurora::TextureProperties::Linear);
	m_GroundTexture->loadTextureData();
}
void TestLayer::OnDetach()
{
	AR_PROFILE_FUNCTION();
}

void TestLayer::OnUpdate(Aurora::TimeStep ts)
{
	AR_PROFILE_FUNCTION();
	
	Aurora::RenderCommand::setClearColor(m_Color);
	Aurora::RenderCommand::Clear();

	Aurora::Renderer3D::ResetStats();

	Aurora::Renderer3D::BeginScene(m_Camera);

	{
		AR_PROFILE_SCOPE("Rendering");
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
		Aurora::Renderer3D::DrawRotatedQuad({ -5.5f, -1.5f, -6.0f }, { m_Rotations.x, m_Rotations.y, rotation }, {3.0f, 3.0f, 3.0f}, glm::vec4{ 0, 247.0f/255.0f, 168.0f/255.0f, 0.7f });
		//Aurora::Renderer3D::DrawRotatedQuad(m_Transalations, m_Rotations, m_Scales, m_CheckerTexture, 20.0f, m_UniColor);
		//Aurora::Renderer3D::DrawQuad({ -1.0f,  0.0f, -8.0f }, { 0.8f, 0.8f, 1.0f }, { 0.8f, 0.2f, 0.3f, 1.0f });
		//Aurora::Renderer3D::DrawQuad({  1.5f, -0.5f, -8.0f }, { 0.5f, 0.75f, 1.0f }, { 0.2f, 0.3f, 0.8f, 1.0f });
		//Aurora::Renderer3D::DrawQuad({ 1.0f,  -1.0f,  0.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, 1);
	}

	Aurora::Renderer3D::EndScene();

	m_Camera.OnUpdate(ts);
}

void TestLayer::OnEvent(Aurora::Event& e)
{
	m_Camera.OnEvent(e);
}

void TestLayer::OnImGuiRender()
{
	AR_PROFILE_FUNCTION();

	Aurora::Application& app = Aurora::Application::GetApp(); // Currently imgui does nothing since its input is not passed on

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