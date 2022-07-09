#pragma once

// Bottleneck currently is all the maths being done in the drawquads functions

#include <Aurora.h>

#include <ImGui/imgui.h>

class TestLayer : public Layer
{
public:
	TestLayer();
	virtual ~TestLayer() = default;

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onImGuiRender() override;
	virtual void onUpdate(TimeStep ts) override;
	virtual void onEvent(Event& e) override;


private:
	Ref<OrthoGraphicCamera> m_OrthoCamera;
	Ref<EditorCamera> m_Camera;

	Ref<Texture> m_CheckerTexture;
	Ref<Texture> m_QiyanaTexture;

private: // ImGui stuff
	bool m_Perspective = true;
	float m_Peak = 0;

	glm::vec4 m_Color = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
	glm::vec4 m_UniColor = glm::vec4(1.0f);

	glm::vec3 m_Transalations = glm::vec3(0.0f, 0.0f, -8.0f);
	glm::vec3 m_Rotations = glm::vec3(0.0f);
	glm::vec3 m_Scales = glm::vec3(1.0f);

	glm::vec3 m_WallTransalations = glm::vec3(0.0f);
	glm::vec3 m_WallRotations = glm::vec3(0.0f, 90.0f, 0.0f);
	glm::vec3 m_WallScales = glm::vec3(1.0f, 1.0f, 1.0f);

};