#include "TestLayer.h"

#include <glm/gtc/constants.hpp>
#include <glm/gtx/compatibility.hpp>

TestLayer::TestLayer()
	: Layer("BatchRenderer"),
	m_Camera(CreateRef<EditorCamera>(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f)),
	m_OrthoCamera(CreateRef<OrthoGraphicCamera>(16.0f / 9.0f, -100.0f, 100.0f))
{
}

void TestLayer::onAttach()
{
	PROFILE_FUNCTION();

	m_CheckerTexture = Texture::Create("resources/textures/checkerboard2.png");
	m_CheckerTexture->flipTextureVertically(true);
	m_CheckerTexture->setTextureWrapping(GL_REPEAT);
	m_CheckerTexture->setTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	m_CheckerTexture->loadTextureData();

	m_QiyanaTexture = Texture::Create("resources/textures/Qiyana.jpg");
	m_QiyanaTexture->flipTextureVertically(true);
	m_QiyanaTexture->setTextureWrapping(GL_REPEAT);
	m_QiyanaTexture->setTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	m_QiyanaTexture->loadTextureData();
}
void TestLayer::onDetach()
{
	PROFILE_FUNCTION();
}

void TestLayer::onUpdate(TimeStep ts)
{
	PROFILE_FUNCTION();
	
	{
		PROFILE_SCOPE("Clear Colors");
		RenderCommand::setClearColor(m_Color);
		RenderCommand::Clear();
	}

	if (m_Perspective)
		Renderer3D::BeginScene(m_Camera);
	else
		Renderer3D::BeginScene(m_OrthoCamera);

	glm::vec3 trans = m_Transalations;

	{
		PROFILE_SCOPE("Rendering");
		// Renderer3D::DrawRotatedQuad(m_Transalations, m_Rotations, m_Scales, m_UniColor);
		Renderer3D::DrawQuad({ -5.0f, -5.0f, -0.1f }, { 10.0f, 10.0f, 0.0f }, m_CheckerTexture, 20.0f);
		Renderer3D::DrawQuad({ -2.0f, -2.0f,  2.0f }, { 2.0f, 2.0f, 2.0f }, m_QiyanaTexture);
		for (float y = -5.0f; y < 5.0f; y += 0.5f)
		{
			for (float x = -5.0f; x < 5.0f; x += 0.5f)
			{
				glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f };
				Renderer3D::DrawQuad({ x, y, 0.0f }, { 0.45f, 0.45f, 0.0f }, color);
			}
		}
		// Renderer3D::DrawRotatedQuad({ 0.0f, 0.0f, 6.0f }, m_Rotations, { 3.0f, 3.0f, 3.0f }, { 0.2f, 0.3f, 0.8f, 1.0f });
		// Renderer3D::DrawRotatedQuad({ 0.0f, 0.0f, 3.0f }, m_Rotations, { 3.0f, 3.0f, 3.0f }, { 0.5f, 0.7f, 0.1f, 1.0f });
		// Renderer3D::DrawRotatedQuad({ 0.0f, 0.0f, 0.0f }, m_Rotations, { 3.0f, 3.0f, 3.0f }, { 0.9f, 0.25f, 0.55f, 1.0f });
		// Renderer3D::DrawRotatedQuad({ 0.0f, 0.0f,-3.0f }, m_Rotations, { 3.0f, 3.0f, 3.0f }, { 0.2f, 1.0f, 0.5f, 1.0f });
		Renderer3D::DrawQuad({ -1.0f,  0.0f, 0.0f }, { 0.8f, 0.8f, 1.0f }, { 0.8f, 0.2f, 0.3f, 1.0f });
		Renderer3D::DrawQuad({  0.5f, -0.5f, 0.0f }, { 0.5f, 0.75f, 1.0f }, { 0.2f, 0.3f, 0.8f, 1.0f });

		if (m_IsR) {
			for (float i = -8.0f; i < 8.0f; i += 1.0f)
			{
				for (float j = -4.5f; j < 4.5f; j += 1.0f)
				{
					Renderer3D::DrawQuad({ i, j, -10.0f }, { 0.8f, 0.8f, 1.0f }, { i / 8.0f, j / 4.5f, 1.0f, 1.0f });
				}
			}

			for (float i = -8.0f; i < 8.0f; i += 1.0f)
			{
				for (float j = -4.5f; j < 4.5f; j += 1.0f)
				{
					Renderer3D::DrawQuad({ i, j + 9.0f, -10.0f }, { 0.8f, 0.8f, 1.0f }, { i / 8.0f, j / 4.5f, 0.5f, 1.0f });
				}
			}

			for (float i = -8.0f; i < 8.0f; i += 1.0f)
			{
				for (float j = -4.5f; j < 4.5f; j += 1.0f)
				{
					Renderer3D::DrawQuad({ i, j - 9.0f, -10.0f }, { 0.8f, 0.8f, 1.0f }, { i / 8.0f, j / 4.5f, 0.0f, 1.0f });
				}
			}

			for (float i = -8.0f; i < 8.0f; i += 1.0f)
			{
				for (float j = -4.5f; j < 4.5f; j += 1.0f)
				{
					Renderer3D::DrawQuad({ i - 16.0f, j - 9.0f, -10.0f }, { 0.8f, 0.8f, 1.0f }, { j / 4.5f, 0.7f, i / 8.0f, 1.0f });
				}
			}

			for (float i = -8.0f; i < 8.0f; i += 1.0f)
			{
				for (float j = -4.5f; j < 4.5f; j += 1.0f)
				{
					Renderer3D::DrawQuad({ i + 16.0f, j - 9.0f, -10.0f }, { 0.8f, 0.8f, 1.0f }, { 0.3f, j / 4.5f, i / 8.0f, 1.0f });
				}
			}

			for (float i = -8.0f; i < 8.0f; i += 1.0f)
			{
				for (float j = -4.5f; j < 4.5f; j += 1.0f)
				{
					Renderer3D::DrawQuad({ i - 16.0f, j + 9.0f, -10.0f }, { 0.8f, 0.8f, 1.0f }, { j / 4.5f, 0.2f, i / 8.0f + 0.3f, 1.0f });
				}
			}

			for (float i = -8.0f; i < 8.0f; i += 1.0f)
			{
				for (float j = -4.5f; j < 4.5f; j += 1.0f)
				{
					Renderer3D::DrawQuad({ i + 16.0f, j + 9.0f, -10.0f }, { 0.8f, 0.8f, 1.0f }, { j / 4.5f - 0.1f, 0.7f, i / 8.0f, 1.0f });
				}
			}
		}
	}

	Renderer3D::EndScene();

	if (m_Perspective)
		m_Camera->OnUpdate(ts);
	else
		m_OrthoCamera->OnUpdate(ts);
}

void TestLayer::onEvent(Event& e)
{
	if (m_Perspective)
		m_Camera->OnEvent(e);
	else
		m_OrthoCamera->OnEvent(e);

	if (Input::isKeyPressed(GLFW_KEY_R))
		m_IsR = !m_IsR;
}

void TestLayer::onImGuiRender()
{
	PROFILE_FUNCTION();

	Application& app = Application::getApp(); // Currently imgui does nothing since its input is not passed on

	ImGui::Begin("Editing Panel");
	if (ImGui::CollapsingHeader("Cube")) {
		ImGui::ColorEdit3("Uniform Color", (float*)&m_UniColor);
		ImGui::SliderFloat3("Cube Translation", (float*)&m_Transalations, -5.0f, 5.0f);
		ImGui::SliderFloat3("Cube Rotations", (float*)&m_Rotations, 0.0f, 360.0f);
		ImGui::SliderFloat3("Cube Scale", (float*)&m_Scales, 0.0f, 3.0f);
	}

	ImGui::Separator();

	ImGui::ColorEdit3("Clear Color", (float*)&m_Color);
	ImGui::Text("Framerate: %.f", ImGui::GetIO().Framerate);

	ImGui::Separator();
	//ImGui::ShowDemoWindow(); // For reference

	ImGui::Checkbox("Camera Type:", &m_Perspective);
	ImGui::SameLine();
	if (m_Perspective)
		ImGui::Text("Perspective Camera!");
	else
		ImGui::Text("OrthoGraphic Camera!");


	ImGui::Checkbox("V Sync ", &(app.getVSync()));

	ImGui::End();
}