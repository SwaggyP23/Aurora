#pragma once
#include <Aurora.h>

#include "Panels/SceneHierarchyPanel.h"

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
		Ref<Aurora::Texture> m_ContainerTexture;
		Ref<Aurora::Texture> m_QiyanaTexture;
		Ref<Aurora::Texture> m_GroundTexture;
		Ref<Aurora::Texture> m_CheckerTexture;

	private: // Main private stuff for this editor
		EditorCamera m_EditorCamera;
		Ref<Framebuffer> m_Framebuffer;
		Ref<Scene> m_ActiveScene;

		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };

		bool m_ViewPortFocused = false;
		bool m_ViewPortHovered = false;
		bool m_Performance = false;
		float m_Peak = 0;

		glm::vec4 m_Color = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);

		// Panels...
		SceneHierarchyPanel	m_SceneHierarchyPanel;
	};

}
