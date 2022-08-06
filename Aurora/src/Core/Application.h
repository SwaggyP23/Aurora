#pragma once

#include "Base.h"
#include "Events/Events.h"
#include "Events/ApplicationEvents.h"
#include "Layers/LayerStack.h"
#include "TimeStep.h"
#include "Window.h"

#include "ImGui/ImGuiLayer.h"

int main(int argc, char** argv);

namespace Aurora {

	struct ApplicationSpecification
	{
		// Name of the application
		std::string Name = "Aurora Application";

		// Size of the application window
		uint32_t WindowWidth = 1280;
		uint32_t WindowHeight = 720;

		// Decorated window
		bool WindowDecorated = true;

		// V-Sync
		bool VSync = true;

		// Start the window in maximized mode
		bool StartMaximized = false;

		// This makes the window not resizable. Note: Better to set to true if StartMaximized is set to false!
		bool SetWindowResizable = false;

		// TODO: Set working directory
		std::string WorkingDirectory;

		// Sets the path for the application's icon
		std::string ApplicationWindowIconPath = "../Resources/Icons/AuroraIcon1.png";
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& specification);
		virtual ~Application();

		void Close();
		void Restart();

		void RenderImGui();

		virtual void OnInit() {}
		virtual void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* layer);
		void ProcessEvents();

		inline float GetCPUTime() const { return m_CPUTime; }
		inline long double GetTimeSinceStart() const { return m_TimeSinceStart; }
		inline ImGuiLayer* GetImGuiLayer() const { return m_ImGuiLayer; }
		inline Window& GetWindow() const { return *m_Window; }
		inline static Application& GetApp() { return *s_Instance; }
		inline const ApplicationSpecification& GetSpecification() const { return m_Specification; }

	private:
		void Run();
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
		bool OnWindowMinimize(WindowMinimizeEvent& e);
		bool OnWindowMaximize(WindowMaximizeEvent& e);

	private:
		ApplicationSpecification m_Specification;
		Scope<Window> m_Window;

		ImGuiLayer* m_ImGuiLayer;
		LayerStack m_LayerStack;

		float m_FrameTime = 0.0f;
		long double m_TimeSinceStart = 0.0f;
		float m_CPUTime = 0.0f;
		TimeStep m_Timestep;

		bool m_Running = true;
		bool m_Minimized = false;

	private:
		static Application* s_Instance;
		friend int ::main(int argc, char** argv);

	};

	// To be defined by user and then the name of the app is specified in the SandBox App when inheriting Application.
	Application* CreateApplication(int argc, char** argv);

}