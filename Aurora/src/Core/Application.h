#pragma once

#include "Graphics/Window.h"
#include "Core/TimeStep.h"
#include "Events/Events.h"
#include "Events/ApplicationEvents.h"
#include "Layers/LayerStack.h"
#include "ImGui/ImGuiLayer.h"

namespace Aurora {

	class Application
	{
	public:
		Application(const std::string& name = "Aurora Engine");
		virtual ~Application();

		void Close();

		void Run();
		void OnEvent(Event& e);

		void pushLayer(Layer* layer);
		void pushOverlay(Layer* layer);

		inline bool& getVSync() { return m_VSync; }

		inline ImGuiLayer* GetImGuiLayer() const { return m_ImGuiLayer; }
		inline Window& getWindow() const { return *m_Window; }
		inline static Application& GetApp() { return *s_Instance; }

	private:
		bool onWindowClose(WindowCloseEvent& e);
		bool onWindowResize(WindowResizeEvent& e);

	private:
		Ref<Window> m_Window;

		ImGuiLayer* m_ImGuiLayer;
		LayerStack m_LayerStack;

		float m_LastFrame = 0.0f;

		bool m_Running = true;
		bool m_Minimized = false;
		bool m_VSync = true;

		static Application* s_Instance;
	};

	// To be defined by user and then the name of the app is specified in the SandBox App when inheriting Application.
	Application* CreateApplication();

}