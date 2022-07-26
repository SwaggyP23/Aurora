#pragma once
#include <Aurora.h>

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


	// Scene Hierarchy Panel
	private:
		void SetContext(const Ref<Scene>& context); // The context for this panel is the scene since it displays the scene's contents
		void ShowSceneHierarchyUI();

		Ref<Scene> m_Context;
		Entity m_SelectionContext;
		int m_NameCounter = 0;

	// Components/Properties Panel
	private:
		void ShowComponentsUI();
		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);
		void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float colomnWidth = 100.0f, float min = 0.0f, float max = 0.0f);

	// Renderer Info/Stats Panels
	private:
		void ShowRendererStatsUI(); // This will eventually be moved to become a panel that is optionally displayed and not mandatory
		void ShowRendererVendorInfoUI();
		//void ShowRendererOverlay(); // To be implemented later

		bool m_ShowRendererVendorInfo = false;
		//bool m_ShowRendererOverlay = false;

	// Performance Panel
	private:
		void ShowPerformanceUI();

		bool m_ShowPerformance = false;
		float m_Peak = 0;

	// This is the main style editing panel and everything such as fonts and such child it
	private:
		void ShowEditPanelUI();

		bool m_ShowEditingPanel = false;

	private:
		void ShowFontPickerUI();

		std::string m_SelectedFontName = "OpenSans, Medium";

	private:
		void ShowPanelPropertiesUI();

	private: // Primary Panels for the editor
		void EnableDocking();
		void ShowMenuBarItems();
		void ShowViewport();

		EditorCamera m_EditorCamera;
		Ref<Framebuffer> m_Framebuffer;
		Ref<Scene> m_ActiveScene;

		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };

		bool m_ViewPortFocused = false;
		bool m_ViewPortHovered = false;
		bool m_ShowDearImGuiDemoWindow = false;

		glm::vec4 m_Color = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);

	};

}
