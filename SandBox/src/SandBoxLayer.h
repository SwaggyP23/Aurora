#pragma once

#include <OpenGLSeries.h>

#include <ImGui/imgui.h>

class SandBoxLayer : public Layer
{
public:
	SandBoxLayer();
	virtual ~SandBoxLayer() = default;

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onImGuiRender() override;
	virtual void onUpdate(TimeStep ts) override;
	virtual void onEvent(Event& e) override;


private:
	Ref<VertexArray> m_VertexArray;
	Ref<VertexArray> m_GroundVertexArray;
	Ref<VertexArray> m_LightVertexArray;
	Ref<VertexArray> m_SphereVertexArray;

	ShaderLibrary m_Shaders; // This can not live on the heap since it will throw a Hash & Mask error

	Ref<OrthoGraphicCamera> m_OrthoCamera;
	Ref<EditorCamera> m_Camera;

	std::vector<Ref<Texture>> m_Textures;

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

	glm::vec4 m_Color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // Initial clear color for lighting.
	glm::vec4 m_LightColor = glm::vec4(1.0f); // Initial clear color for lighting.
	glm::vec4 m_UniColor = glm::vec4(1.0f);

};