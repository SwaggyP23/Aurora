#pragma once

#include "Base.h"
#include "Events/Events.h"
#include "Events/ApplicationEvents.h"
#include "LayerStack.h"
#include "TimeStep.h"
#include "Debugging/Timer.h"
#include "Window.h"
#include "Renderer/Renderer.h"

#include "ImGui/ImGuiLayer.h"

namespace Aurora {

	struct ApplicationSpecification
	{
		// Name of the application
		std::string Name = "Aurora Application";

		// Size of the application window
		uint32_t WindowWidth = 1280;
		uint32_t WindowHeight = 720;

		// Creates a full screen window
		bool Fullscreen = false;

		// Decorated window
		bool WindowDecorated = true;

		// V-Sync
		bool VSync = true;

		// Start the window in maximized mode
		bool StartMaximized = false;

		// This makes the window not resizable. Note: Better to set to true if screen is not maximized
		bool SetWindowResizable = true;

		// Controls whether imgui is enabled or not, this is useful for runtime applications
		bool EnableImGui = true;

		// TODO: Set working directory
		std::string WorkingDirectory;

		// Sets the path for the application's icon
		std::string ApplicationWindowIconPath = "../Resources/Icons/AuroraIcon1.png";

		RendererConfig RendererConfiguration;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& specification);
		virtual ~Application();

		void Close();

		void RenderImGui();

		// These are functions that could be ovverriden by the user
		virtual void OnInit() {}
		virtual void OnShutdown();
		virtual void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* layer);
		void ProcessEvents();

		[[nodiscard]] inline float GetCPUTime() const { return m_CPUTime; }
		[[nodiscard]] inline float GetFrameTime() const { return m_FrameTime; }
		[[nodiscard]] inline TimeStep GetTimeStep() const { return m_Timestep; }
		inline void SetTickDeltaTime(float delta) { m_TickDelta = delta; }

		[[nodiscard]] inline ImGuiLayer* GetImGuiLayer() const { return m_ImGuiLayer; }

		[[nodiscard]] inline Window& GetWindow() { return *m_Window; }

		[[nodiscard]] inline static Application& GetApp() { return *s_Instance; }
		[[nodiscard]] inline const ApplicationSpecification& GetSpecification() const { return m_Specification; }

		[[nodiscard]] inline PerformanceProfiler* GetPerformanceProfiler() const { return m_Profiler; }

	private:
		void Run();
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
		bool OnWindowMinimize(WindowMinimizeEvent& e);

	private:
		ApplicationSpecification m_Specification;
		Scope<Window> m_Window;

		ImGuiLayer* m_ImGuiLayer;
		LayerStack m_LayerStack;

		float m_FrameTime = 0.0f;
		float m_LastFrameTime = 0.0f;
		float m_CPUTime = 0.0f;
		float m_TickDelta = 1.0f;
		TimeStep m_Timestep;
		PerformanceProfiler* m_Profiler = nullptr; // TODO: Should be null in Dist

		bool m_Running = true;
		bool m_Minimized = false;

	private:
		static Application* s_Instance;
		friend int Main(int argc, char** argv);

	};

	// To be defined by user and then the name of the app is specified in the SandBox App when inheriting Application.
	Application* CreateApplication(int argc, char** argv);

}