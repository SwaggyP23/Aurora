#pragma once

#include "Base.h"
#include "Window.h"
#include "Layers/LayerStack.h"
#include "Events/Events.h"
#include "Events/ApplicationEvents.h"
#include "TimeStep.h"
#include "ImGui/ImGuiLayer.h"

int main(int argc, char** argv);

namespace Aurora {

	struct ApplicationSpecification
	{
		std::string Name = "Aurora Application";
		uint32_t WindowWidth = 1280;
		uint32_t WindowHeight = 720;
		bool WindowDecorated = true;
		bool VSync = true;
		bool StartMaximized = false;
		bool SetWindowResizable = false;

		std::string WorkingDirectory;
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
		inline float GetTimeSinceStart() const { return m_TimeSinceStart; }
		inline ImGuiLayer* GetImGuiLayer() const { return m_ImGuiLayer; }
		inline Window& GetWindow() const { return *m_Window; }
		inline static Application& GetApp() { return *s_Instance; }
		inline const ApplicationSpecification& GetSpecification() const { return m_Specification; }

	private:
		void Run();
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
		//bool OnWindowMinimize(WindowMinimizeEvent& e); // Should find a way to properly dispatch the window minimize events

	private:
		ApplicationSpecification m_Specification;
		Scope<Window> m_Window;

		ImGuiLayer* m_ImGuiLayer;
		LayerStack m_LayerStack;

		float m_FrameTime = 0.0f;
		float m_TimeSinceStart = 0.0f;
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