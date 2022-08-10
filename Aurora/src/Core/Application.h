#pragma once

#include "Base.h"
#include "Events/Events.h"
#include "Events/ApplicationEvents.h"
#include "Layers/LayerStack.h"
#include "TimeStep.h"
#include "Debugging/Timer.h"
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

		// This acts as the restart also
		void Restart();
		void Close();

		void RenderImGui();

		virtual void OnInit() {}
		virtual void OnShutdown();
		virtual void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* layer);
		void ProcessEvents();

		inline float GetCPUTime() const { return m_CPUTime; }
		inline long double GetLastFrameTime() const { return m_LastFrameTime; }

		inline ImGuiLayer* GetImGuiLayer() const { return m_ImGuiLayer; }

		inline Window& GetWindow() const { return *m_Window; }

		inline static Application& GetApp() { return *s_Instance; }
		inline const ApplicationSpecification& GetSpecification() const { return m_Specification; }

		inline PerformanceProfiler* GetPerformanceProfiler() const { return m_Profiler; }

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
		long double m_LastFrameTime = 0.0f;
		float m_CPUTime = 0.0f;
		TimeStep m_Timestep;
		PerformanceProfiler* m_Profiler = nullptr; // TODO: Should be null in Dist

		bool m_Restart = false;
		bool m_Running = true;
		bool m_Minimized = false;

	private:
		static Application* s_Instance;
		friend int Main(int argc, char** argv);

	};

	// To be defined by user and then the name of the app is specified in the SandBox App when inheriting Application.
	Application* CreateApplication(int argc, char** argv);

}