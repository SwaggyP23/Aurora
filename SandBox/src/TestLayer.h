#pragma once

// Bottleneck currently is all the maths being done in the drawquads functions

#include <Aurora.h>

#include <ImGui/imgui.h>

class TestLayer : public Aurora::Layer
{
public:
	TestLayer();
	virtual ~TestLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnImGuiRender() override;
	virtual void OnUpdate(Aurora::TimeStep ts) override;
	virtual void OnEvent(Aurora::Event& e) override;


private:
	Aurora::Ref<Aurora::EditorCamera> m_Camera;
	Aurora::Ref<Aurora::Shader> modelShader;

	Aurora::Ref<Aurora::Texture> m_ContainerTexture;
	Aurora::Ref<Aurora::Texture> m_QiyanaTexture;
	Aurora::Ref<Aurora::Texture> m_GroundTexture;

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