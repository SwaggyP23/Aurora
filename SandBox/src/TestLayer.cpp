#include "TestLayer.h"

TestLayer::TestLayer()
	: Layer("BatchRenderer"),
	m_Camera(CreateRef<EditorCamera>(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f)),
	m_OrthoCamera(CreateRef<OrthoGraphicCamera>(16.0f / 9.0f, -100.0f, 100.0f))
{
}

void TestLayer::onAttach()
{
	PROFILE_FUNCTION();

	m_Texture = Texture::Create("resources/textures/checkerboard.png");
	m_Texture->flipTextureVertically(true);
	m_Texture->setTextureWrapping(GL_REPEAT);
	m_Texture->setTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	m_Texture->loadTextureData();
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
		Renderer3D::DrawRotatedQuad(m_Transalations, m_Rotations, m_Scales, m_UniColor);
		Renderer3D::DrawRotatedQuad({ 0.0f, 0.0f,-4.6f }, glm::vec3(0.0f), { 10.0f * 20.0f, 10.0f * 20.0f, 0.0f}, m_Texture);
		Renderer3D::DrawRotatedQuad({ 0.0f, 0.0f, 6.0f }, m_Rotations, { 3.0f, 3.0f, 3.0f }, { 0.2f, 0.3f, 0.8f, 1.0f });
		Renderer3D::DrawRotatedQuad({ 0.0f, 0.0f, 3.0f }, m_Rotations, { 3.0f, 3.0f, 3.0f }, { 0.5f, 0.7f, 0.1f, 1.0f });
		Renderer3D::DrawRotatedQuad({ 0.0f, 0.0f, 0.0f }, m_Rotations, { 3.0f, 3.0f, 3.0f }, { 0.9f, 0.25f, 0.55f, 1.0f });
		Renderer3D::DrawRotatedQuad({ 0.0f, 0.0f,-3.0f }, m_Rotations, { 3.0f, 3.0f, 3.0f }, { 0.2f, 1.0f, 0.5f, 1.0f });
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
		LOG_TRACE("WASSUP BRO!!");
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