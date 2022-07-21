#include "GameLayer.h"

GameLayer::GameLayer()
	: Layer("BatchRenderer"),
	m_Camera(Aurora::EditorCamera(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f))
{
}

void GameLayer::OnAttach()
{
	AR_PROFILE_FUNCTION();

	m_Texture = Aurora::Texture::Create("resources/textures/Pepsi.png");
	m_Texture->flipTextureVertically(true);
	m_Texture->setTextureWrapping(Aurora::TextureProperties::Repeat);
	m_Texture->setTextureFiltering(Aurora::TextureProperties::MipMap_LinearLinear, Aurora::TextureProperties::Linear);
	m_Texture->loadTextureData();

	m_AppTexture = Aurora::Texture::Create("resources/textures/apple.png");
	m_AppTexture->flipTextureVertically(true);
	m_AppTexture->setTextureWrapping(Aurora::TextureProperties::Repeat);
	m_AppTexture->setTextureFiltering(Aurora::TextureProperties::MipMap_LinearLinear, Aurora::TextureProperties::Linear);
	m_AppTexture->loadTextureData();

	m_SnakeTexture = Aurora::Texture::Create("resources/textures/Snake.png");
	m_SnakeTexture->flipTextureVertically(true);
	m_SnakeTexture->setTextureWrapping(Aurora::TextureProperties::Repeat);
	m_SnakeTexture->setTextureFiltering(Aurora::TextureProperties::MipMap_LinearLinear, Aurora::TextureProperties::Linear);
	m_SnakeTexture->loadTextureData();
}

void GameLayer::OnDetach()
{
	AR_PROFILE_FUNCTION();
}

void GameLayer::OnUpdate(Aurora::TimeStep ts)
{
	AR_PROFILE_FUNCTION();

	Aurora::RenderCommand::setClearColor(m_Color);
	Aurora::RenderCommand::Clear();

	Aurora::Renderer3D::BeginScene(m_Camera);

	//glm::vec3 trans = m_Transalations;

	Aurora::Renderer3D::DrawQuad({ 0.0f, 0.0f, -0.55f }, { PANELWIDTH, PANELHEIGHT, 0.0f }, m_Texture);


	if (!m_Generated) {
		m_X = Aurora::Utils::Random::Float(), m_Y = Aurora::Utils::Random::Float();
		m_Generated = true;
	}
	Aurora::Renderer3D::DrawQuad({ m_X, m_Y, 0.0f }, { 1.0f, 1.0f, 0.5f }, m_AppTexture, 1.0f);

	if (!m_GameOver)
	{
		std::vector<BodyPart> vec = m_Snake.getBodyParts();
		//LOG_WARN("Vec[0]: {0}, {1}", vec[0].position.x, vec[0].position.y);
		for (auto& part : vec)
		{
			glm::vec2 pos = part.position;
			Aurora::Renderer3D::DrawQuad({ pos.x, pos.y, 0.0f }, { 1.0f, 1.0f, 0.5f }, m_SnakeTexture, 0.5f);
		}
	}

	Aurora::Renderer3D::EndScene();

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

	m_Camera.OnUpdate(ts);
}

void GameLayer::OnEvent(Aurora::Event& e)
{
	if (Aurora::Input::isKeyPressed(Aurora::Key::W) && !down) {
		m_Dir = Direction::Up;
		up = true;
		left = false;
		right = false;
	}
	else if (Aurora::Input::isKeyPressed(Aurora::Key::A) && !right) {
		m_Dir = Direction::Left;
		left = true;
		up = false;
		down = false;
	}
	else if (Aurora::Input::isKeyPressed(Aurora::Key::S) && !up) {
		m_Dir = Direction::Down;
		down = true;
		right = false;
		left = false;
	}
	else if (Aurora::Input::isKeyPressed(Aurora::Key::D) && !left) {
		m_Dir = Direction::Right;
		right = true;
		up = false;
		down = false;
	}

	if (Aurora::Input::isKeyPressed(Aurora::Key::LeftAlt)) // This is for debugging
		m_Snake.IncParts();

	m_Camera.OnEvent(e);
}

void GameLayer::OnImGuiRender()
{
	AR_PROFILE_FUNCTION();

	Aurora::Application& app = Aurora::Application::GetApp(); // Currently imgui does nothing since its input is not passed on

	ImGui::Begin("Editing Panel");
	if (ImGui::CollapsingHeader("Cube")) {

	}

	ImGui::Separator();

	ImGui::ColorEdit3("Clear Color", (float*)&m_Color);
	ImGui::Text("Framerate: %.f", ImGui::GetIO().Framerate);

	ImGui::Separator();
	//ImGui::ShowDemoWindow(); // For reference

	ImGui::Checkbox("V Sync ", &(app.getVSync()));

	ImGui::End();
}