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
		virtual void OnUpdate(Aurora::TimeStep ts) override;
		virtual void OnTick() override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Aurora::Event& e) override;


	// Scene Hierarchy Panel
	private:
		void SetContextForSceneHeirarchyPanel(const Ref<Scene>& context); // The context for this panel is the scene since it displays the scene's contents
		void ShowSceneHierarchyUI();

		Ref<Scene> m_Context;
		Entity m_SelectionContext;
		Entity m_GroundEntity;
		Entity m_HoveredEntity;

	// Components/Properties Panel
	private:
		void ShowComponentsUI();
		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);
		void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float colomnWidth = 100.0f, float min = 0.0f, float max = 0.0f, float stepValue = 0.1f);

	// Renderer Info/Stats Panels
	private:
		void ShowRendererStatsUI(); // This will eventually be moved to become a panel that is optionally displayed and not mandatory
		void ShowRendererVendorInfoUI();
		//void ShowRendererOverlay(); // To be implemented later

		bool m_ShowRendererVendorInfo = false;
		//bool m_ShowRendererOverlay = false;

	// Performance Panel
	private:
		void ShowTimers();
		void ShowPerformanceUI();

		std::vector<std::tuple<const char*, float>> m_SortedTimerValues;
		bool m_ShowPerformance = true;
		float m_Peak = 0;

	// This is the main style editing panel and everything such as fonts and such child it
	private:
		void ShowEditPanelUI();

		bool m_ShowEditingPanel = false;

	private:
		void ShowFontPickerUI();

		std::string m_SelectedFontName = "OpenSans, Medium";

	// Help Panels and UI
	private:
		void ShowEditorCameraHelpUI();

		bool m_ShowEditorCameraHelpUI = false;
		bool m_ShowDearImGuiDemoWindow = false;

	// File Dialogs and Scene helper functions
	private:
		void NewScene();
		void OpenScene();
		void OpenScene(const std::filesystem::path& path);
		void SaveScene();
		void SaveSceneAs();
		void SerializeScene(const Ref<Scene>& scene, const std::filesystem::path& path);

		std::filesystem::path m_EditorScenePath;

	// Primary Panels for the editor
	private:
		void EnableDocking();
		void ShowMenuBarItems();

		float GetSnapValue();
		void ShowViewport();

		void ManipulateGizmos();

		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

		void ShowSettingsUI();
		void ShowRestartModalUI();
		void ShowCloseModalUI();

		bool m_ShowSettingsUI = false;
		bool m_ShowRestartModal = false;
		bool m_ShowCloseModal = false;

	private:
		EditorCamera m_EditorCamera;
		Ref<Framebuffer> m_Framebuffer;

		Ref<Scene> m_EditorScene;
		Ref<Scene> m_ActiveScene;

		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
		glm::vec2 m_ViewportBounds[2];

		int m_GizmoType = -1;

		bool m_ViewPortFocused = false;
		bool m_ViewPortHovered = false;
		bool m_AllowViewportCameraEvents = false;

		glm::vec4 m_Color = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);

	};

}