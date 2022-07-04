#pragma once

#include <OpenGLSeries.h>

#include <ImGui/imgui.h>

class BatchRenderer : public Layer
{
public:
	BatchRenderer();

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onImGuiRender() override;
	virtual void onUpdate(TimeStep ts) override;
	virtual void onEvent(Event& e) override;


private:
	Ref<VertexArray> m_VertexArray;
	Ref<Shader> m_Shader;

	Ref<OrthoGraphicCamera> m_OrthoCamera;
	Ref<EditorCamera> m_Camera;

	glm::vec4 uniColor = glm::vec4(0.5f);

	Ref<Texture> m_Texture;

private: // ImGui stuff
	float m_Time = 0.0f;

	bool m_Perspective = true;

	glm::vec4 m_Color = glm::vec4(0.0f);
	glm::vec4 m_UniColor = glm::vec4{ 0.2f, 0.3f, 0.8f, 1.0f };

	glm::vec3 m_Transalations = glm::vec3(0.0f);
	glm::vec3 m_Rotations = glm::vec3(0.0f);
	glm::vec3 m_Scales = glm::vec3(1.0f);

};