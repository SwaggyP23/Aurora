#pragma once
#include <Aurora.h>

#include <ImGui/imgui.h>

namespace Aurora {

	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;

		virtual void onAttach() override;
		virtual void onDetach() override;
		virtual void onImGuiRender() override;
		virtual void onUpdate(Aurora::TimeStep ts) override;
		virtual void onEvent(Aurora::Event& e) override;


	private:
		Aurora::Ref<Aurora::EditorCamera> m_Camera;
		Aurora::Ref<Aurora::OrthoGraphicCamera> m_OrthoCamera;

		Aurora::Ref<Aurora::Texture> m_ContainerTexture;
		Aurora::Ref<Aurora::Texture> m_QiyanaTexture;
		Aurora::Ref<Aurora::Texture> m_GroundTexture;
		Aurora::Ref<Aurora::Texture> m_CheckerTexture;

	private: // ImGui stuff
		bool m_Perspective = true;
		float m_Peak = 0;

		glm::vec4 m_Color = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
		glm::vec4 m_UniColor = glm::vec4(1.0f);
	};

}
