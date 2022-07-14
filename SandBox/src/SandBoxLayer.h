#pragma once

#include <Aurora.h>

#include <ImGui/imgui.h>

class SandBoxLayer : public Aurora::Layer
{
public:
	SandBoxLayer();
	virtual ~SandBoxLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnImGuiRender() override;
	virtual void OnUpdate(Aurora::TimeStep ts) override;
	virtual void OnEvent(Aurora::Event& e) override;


private:
	Aurora::Ref<Aurora::VertexArray> m_VertexArray;
	Aurora::Ref<Aurora::VertexArray> m_GroundVertexArray;
	Aurora::Ref<Aurora::VertexArray> m_LightVertexArray;
	Aurora::Ref<Aurora::VertexArray> m_SphereVertexArray;

	Aurora::ShaderLibrary m_Shaders; // This can not live on the heap since it will throw a Hash & Mask error

	Aurora::Ref<Aurora::OrthoGraphicCamera> m_OrthoCamera;
	Aurora::Ref<Aurora::EditorCamera> m_Camera;

	std::vector<Aurora::Ref<Aurora::Texture>> m_Textures;

	bool m_IsRPressed = false;

	glm::vec4 uniColor = glm::vec4(0.5f);

	std::array<glm::vec3, 10> m_CubePositions{};

private: // ImGui stuff
	float m_Time = 0.0f;
	float m_Blend = 0.0f;
	float m_AmbLight = 0.06f;

	bool m_Perspective = true;

	glm::vec3 m_Transalations = glm::vec3(0.0f);
	glm::vec3 m_Rotations = glm::vec3(0.0f);
	glm::vec3 m_Scales = glm::vec3(1.0f);

	glm::vec3 m_LightTranslations = glm::vec3(2.0f);
	glm::vec3 m_LightScales = glm::vec3(0.2f);

	glm::vec3 m_GroundTranslations = glm::vec3(0.0f);
	glm::vec3 m_GroundScales = glm::vec3(1.0f);

	glm::vec3 m_SphereTransalations = glm::vec3(-4.0f, 5.0f, 0.0f);
	glm::vec3 m_SphereRotations = glm::vec3(0.0f);
	glm::vec3 m_SphereScales = glm::vec3(1.0f);

	glm::vec4 m_Color = glm::vec4(0.09f, 0.09f, 0.09f, 1.0f); // Initial clear color for lighting.
	glm::vec4 m_LightColor = glm::vec4(1.0f); // Initial clear color for lighting.
	glm::vec4 m_UniColor = glm::vec4(1.0f);

};