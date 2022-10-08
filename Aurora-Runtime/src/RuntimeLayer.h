#pragma once

#include <Aurora.h>

namespace Aurora {

	class RuntimeLayer : public Layer
	{
	public:
		RuntimeLayer(std::string_view scenePath); // TODO: Switch to projectPath once projects are a thing
		virtual ~RuntimeLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(TimeStep ts) override;
		virtual void OnTick() override;
		virtual void OnEvent(Event& e) override;

		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

		void OpenScene(const std::string& filePath);

	private:
		void OnScenePlay();
		void OnSceneStop();

		void UpdateWindowTitle(const std::string& sceneName);

	private:
		Ref<Scene> m_RuntimeScene;
		Ref<SceneRenderer> m_SceneRenderer;
		Ref<Renderer2D> m_Renderer2D;

		Ref<RenderPass> m_FinalCompositeRenderPass;
		Ref<Material> m_FinalCompositeMaterial;

		std::string m_ScenePath; // TODO: Switch to projectPath once projects are a thing

		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
		uint32_t m_FramesPerSecond = 0;
		float m_FrameTime = 0.0f;
		float m_GPUTime = 0.0f;
		float m_CPUTime = 0.0f;

		// For debugging
		EditorCamera m_EditorCamera;

		bool m_AllowViewportCameraEvents = false;
		bool m_ViewportPanelHovered = false;
		bool m_ViewportPanelFocused = false;


	};

}