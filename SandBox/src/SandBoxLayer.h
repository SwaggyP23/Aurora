#pragma once

#include <OpenGLSeries.h>

#include <ImGui/imgui.h>

class SandBoxLayer : public Layer
{
public:
	SandBoxLayer();

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onImGuiRender() override;
	virtual void onUpdate(/*should take in timestep*/) override;
	virtual void onEvent(Event& e) override;


private:
	Ref<VertexArray> m_VertexArray;
	Ref<VertexArray> m_GroundVertexArray;
	Ref<VertexArray> m_LightVertexArray;
	Ref<VertexArray> m_SphereVertexArray;

	Ref<ShaderLibrary> m_Shaders;

	//Ref<Shader> m_Shader;
	//Ref<Shader> m_LightShader;
	//Ref<Shader> m_GroundShader;
	//Ref<Shader> m_SphereShader;

	Ref<EditorCamera> m_Camera;

	bool m_IsRPressed = false;

	glm::vec4 uniColor = glm::vec4(0.5f);

	std::vector<Ref<Texture>> m_Textures;

	std::array<glm::vec3, 10> m_CubePositions;

private: // ImGui stuff
	float m_Time = 0.0f;
	float m_Blend = 0.0f;
	float m_AmbLight = 0.06f;
	float m_Rotation = 0.0f;

	glm::vec3 m_Transalations = glm::vec3(0.0f);
	glm::vec3 m_Scales = glm::vec3(1.0f);

	glm::vec3 m_LightTranslations = glm::vec3(2.0f);
	glm::vec3 m_LightScales = glm::vec3(0.2f);

	glm::vec3 m_GroundTranslations = glm::vec3(0.0f);
	glm::vec3 m_GroundScales = glm::vec3(1.0f);

	glm::vec3 m_SphereTransalations = glm::vec3(-4.0f, 5.0f, 0.0f);
	glm::vec3 m_SphereRotations = glm::vec3(0.0f);
	glm::vec3 m_SphereScales = glm::vec3(1.0f);

	//glm::vec4 m_Color = glm::vec4(0.2f, 0.6f, 0.6f, 1.0f); // Initial clear color.
	glm::vec4 m_Color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // Initial clear color for lighting.
	glm::vec4 m_LightColor = glm::vec4(1.0f); // Initial clear color for lighting.
	glm::vec4 m_UniColor = glm::vec4(1.0f);

};