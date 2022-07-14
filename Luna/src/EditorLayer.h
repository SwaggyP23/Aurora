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


	private: // Temporary things like textures for now
		Aurora::Ref<Aurora::Texture> m_ContainerTexture;
		Aurora::Ref<Aurora::Texture> m_QiyanaTexture;
		Aurora::Ref<Aurora::Texture> m_GroundTexture;
		Aurora::Ref<Aurora::Texture> m_CheckerTexture;

	private: // Main private stuff for this editor
		Aurora::Ref<Aurora::EditorCamera> m_Camera;
		Aurora::Ref<Aurora::OrthoGraphicCamera> m_OrthoCamera;
		Aurora::Ref<Framebuffer> m_Framebuffer;

		glm::vec2 m_ViewportSize = glm::vec2(0.0f);

		bool m_Perspective = true;
		bool m_ViewPortFocused = false;
		bool m_ViewPortHovered = false;
		float m_Peak = 0;

		glm::vec4 m_Color = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
		glm::vec4 m_UniColor = glm::vec4(1.0f);
	};

}
