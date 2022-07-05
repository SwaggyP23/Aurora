#include "GameLayer.h"

GameLayer::GameLayer()
	: Layer("BatchRenderer"),
	m_Camera(CreateRef<EditorCamera>(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f)),
	m_OrthoCamera(CreateRef<OrthoGraphicCamera>(16.0f / 9.0f, -100.0f, 100.0f))
{
}

void GameLayer::onAttach()
{
	PROFILE_FUNCTION();

	Utils::Random::Init();

	m_Texture = Texture::Create("resources/textures/Pepsi.png");
	m_Texture->flipTextureVertically(true);
	m_Texture->setTextureWrapping(GL_REPEAT);
	m_Texture->setTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	m_Texture->loadTextureData();

	m_AppTexture = Texture::Create("resources/textures/apple.png");
	m_AppTexture->flipTextureVertically(true);
	m_AppTexture->setTextureWrapping(GL_REPEAT);
	m_AppTexture->setTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	m_AppTexture->loadTextureData();

	m_SnakeTexture = Texture::Create("resources/textures/Snake.png");
	m_SnakeTexture->flipTextureVertically(true);
	m_SnakeTexture->setTextureWrapping(GL_REPEAT);
	m_SnakeTexture->setTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	m_SnakeTexture->loadTextureData();
}

void GameLayer::onDetach()
{
	PROFILE_FUNCTION();
}

void GameLayer::onUpdate(TimeStep ts)
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

	//glm::vec3 trans = m_Transalations;

	Renderer3D::DrawQuad({ 0.0f, 0.0f, -0.55f }, { PANELWIDTH, PANELHEIGHT, 0.0f }, m_Texture);


	if (!m_Generated) {
		m_X = Utils::Random::Float(), m_Y = Utils::Random::Float();
		m_Generated = true;
	}
	Renderer3D::DrawQuad({ m_X, m_Y, 0.0f }, { 1.0f, 1.0f, 0.5f }, m_AppTexture, 1.0f);

	if (!m_GameOver)
	{
		std::vector<BodyPart> vec = m_Snake.getBodyParts();
		//LOG_WARN("Vec[0]: {0}, {1}", vec[0].position.x, vec[0].position.y);
		for (auto& part : vec)
		{
			glm::vec2 pos = part.position;
			Renderer3D::DrawQuad({ pos.x, pos.y, 0.0f }, { 1.0f, 1.0f, 0.5f }, m_SnakeTexture, 0.5f);
		}
	}

	Renderer3D::EndScene();

	if (m_Dir == Direction::Up)
		m_Snake.moveUp(ts);
	else if (m_Dir == Direction::Left)
		m_Snake.moveLeft(ts);
	else if (m_Dir == Direction::Down)
		m_Snake.moveDown(ts);
	else if (m_Dir == Direction::Right)
		m_Snake.moveRight(ts);

	glm::vec2 Collision(m_X, m_Y);
	if (m_Snake.checkCollision(Collision))
		m_Generated = false;

	if (m_Perspective)
		m_Camera->OnUpdate(ts);
	else
		m_OrthoCamera->OnUpdate(ts);
}

void GameLayer::onEvent(Event& e)
{
	if (Input::isKeyPressed(GLFW_KEY_W) && !down) {
		m_Dir = Direction::Up;
		up = true;
		left = false;
		right = false;
	}
	else if (Input::isKeyPressed(GLFW_KEY_A) && !right) {
		m_Dir = Direction::Left;
		left = true;
		up = false;
		down = false;
	}
	else if (Input::isKeyPressed(GLFW_KEY_S) && !up) {
		m_Dir = Direction::Down;
		down = true;
		right = false;
		left = false;
	}
	else if (Input::isKeyPressed(GLFW_KEY_D) && !left) {
		m_Dir = Direction::Right;
		right = true;
		up = false;
		down = false;
	}

	if (Input::isKeyPressed(GLFW_KEY_LEFT_ALT)) // This is for debugging
		m_Snake.IncParts();

	if (m_Perspective)
		m_Camera->OnEvent(e);
	else
		m_OrthoCamera->OnEvent(e);
}

void GameLayer::onImGuiRender()
{
	PROFILE_FUNCTION();

	Application& app = Application::getApp(); // Currently imgui does nothing since its input is not passed on

	ImGui::Begin("Editing Panel");
	if (ImGui::CollapsingHeader("Cube")) {

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