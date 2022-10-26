#pragma once
#include <Aurora.h>

#include "Editor/EditorResources.h"
#include "Editor/EditorPanelLibrary.h"

#include <imgui/imgui_internal.h>

namespace Aurora {

	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(TimeStep ts) override;
		virtual void OnTick() override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override;

		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
		bool OnPathDrop(WindowPathDropEvent& e);

		void OnRender2D();

		std::pair<float, float> GetMouseInViewportSpace();
		std::pair<glm::vec3, glm::vec3> CastRay(const EditorCamera& camera, float mx, float my);

	// Renderer Info/Stats Panels
	private:
		void ShowRendererStatsUI(); // This will eventually be moved to become a panel that is optionally displayed and not mandatory
		void ShowRendererVendorInfoUI();
		//void ShowRendererOverlay(); // TODO: to be implemented later

		bool m_ShowRendererVendorInfo = false;
		bool m_ShowRenderStatsUI = true;
		//bool m_ShowRendererOverlay = false; // TODO: to be implemented later

	// Material stuff...
	private:
		void ShowMaterialsPanel();

		bool m_ShowMaterialsPanel = true;

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
		void ShowScreenshotPanel();
		void TakeScreenShotOfOpenScene();

		bool m_ShowEditingPanel = false;
		bool m_ShowScreenshotPanel = false;
		Ref<Texture2D> m_DisplayImage = nullptr;

	private:
		void ShowFontPickerUI();

	// Help Panels and UI
	private:
		void ShowEditorCameraHelpUI();

		bool m_ShowEditorCameraHelpUI = false;

		bool m_ShowDearImGuiMetricsWindow = false;
		bool m_ShowDearImGuiStackToolWindow = false;
		bool m_ShowDearImGuiDebugLogWindow = false;

	// File Dialogs and Scene helper functions
	private:
		void NewScene();
		void OpenScene();
		void OpenScene(const std::filesystem::path& path);
		void SaveScene();
		void SaveSceneAs();
		void SerializeScene(const Ref<Scene>& scene, const std::filesystem::path& path);

		void OnScenePlay();
		void OnSceneSimulate(); // TODO: Implement when we have physics
		void OnSceneStop();

		std::string m_SceneFilePath;

	// Primary Panels for the editor
	private:
		void EnableDocking();
		void ShowMenuBarItems();

		float GetSnapValue();
		void ShowViewport();

		void DrawGizmos();
		void DrawGizmosToolBar();
		void DrawCentralBar();

		void ShowSettingsUI();
		void ShowCloseModalUI();

		void OnEntityDeleted(Entity entity);

		bool m_ShowSettingsUI = false;
		bool m_ShowCloseModal = false;

	private:
		Scope<EditorPanelsLibrary> m_PanelsLibrary;

		EditorCamera m_EditorCamera;

		Ref<Renderer2D> m_Renderer2D;

		struct SelectionData
		{
			Aurora::Entity Entity;
			SubMesh* Mesh = nullptr;
			float Distance = 0.0f;
		};

		Ref<Scene> m_EditorScene;
		Ref<Scene> m_RuntimeScene;
		Ref<Scene> m_ActiveScene;
		
		Ref<SceneRenderer> m_ViewportRenderer;

		ImRect m_ViewportRect;
		ImVec2 m_ViewportSize = { 0.0f, 0.0f };
		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;

		int32_t m_GizmoType = -1;

		float m_LineWidth = 2.0f;

		enum class SceneState
		{
			Edit = 0, Play = 1, Pause = 2, Simulate = 3
		};

		SceneState m_SceneState = SceneState::Edit;

		bool m_ShowGizmos = true;
		bool m_ShowIcons = true;
		bool m_ShowBoundingBoxes = false;

		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;
		bool m_AllowViewportCameraEvents = false;
		bool m_StartedRightClickInViewport = false;
		bool m_EditorCameraInRuntime = false;

	};

}