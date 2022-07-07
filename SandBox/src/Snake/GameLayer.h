#pragma once

#include <OpenGLSeries.h>

#include <ImGui/imgui.h>
#include "Snake.h"

class GameLayer : public Layer
{
public:
	GameLayer();
	virtual ~GameLayer() = default;

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onImGuiRender() override;
	virtual void onUpdate(TimeStep ts) override;
	virtual void onEvent(Event& e) override;


private:
	Ref<OrthoGraphicCamera> m_OrthoCamera;
	Ref<EditorCamera> m_Camera;

	Ref<Texture> m_Texture;
	Ref<Texture> m_AppTexture;
	Ref<Texture> m_SnakeTexture;

private: // ImGui stuff
	bool m_Perspective = true;
	bool m_GameOver = false;
	bool m_Eaten = false;
	bool m_Generated = false;

	bool left = false, right = false, up = false, down = false;
	
	float m_X = 0.0f, m_Y = 0.0f;

	enum class Direction
	{
		None = 0, Up, Down, Left, Right
	};

	Direction m_Dir = Direction::None;

	Snake m_Snake;

	glm::vec4 m_Color = glm::vec4(0.123f, 0.123f, 0.123f, 1.0f);
	glm::vec4 m_UniColor = glm::vec4{ 0.2f, 0.3f, 0.8f, 1.0f };

	glm::vec3 m_Transalations = glm::vec3(0.0f);
	glm::vec3 m_Rotations = glm::vec3(0.0f);
	glm::vec3 m_Scales = glm::vec3(1.0f);

};