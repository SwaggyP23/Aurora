#pragma once
#include <Aurora.h>

#include <ImGui/imgui.h>

namespace Aurora {

	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;
		virtual void OnUpdate(Aurora::TimeStep ts) override;
		virtual void OnEvent(Aurora::Event& e) override;


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
		bool m_Performance = false;
		float m_Peak = 0;

		glm::vec4 m_Color = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
		glm::vec4 m_UniColor = glm::vec4(1.0f);
	};

}
