#pragma once

#include "Graphics/Window.h"
#include "Core/TimeStep.h"
#include "Events/Events.h"
#include "Events/ApplicationEvents.h"
#include "Layers/LayerStack.h"
#include "ImGui/ImGuiLayer.h"

class Application
{
public:
	Application(const std::string& name = "OpenGLSeries App");
	virtual ~Application();

	void Run();
	void onEvent(Event& e);

	void pushLayer(Layer* layer);
	void pushOverlay(Layer* layer);
	
	inline bool& getVSync() { return m_VSync; }

	inline ImGuiLayer* getImGuiLayer() const { return m_ImGuiLayer; }
	inline Window& getWindow() const { return *m_Window; }
	inline static Application& getApp() { return *s_Instance; }

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

// To be defined by user in sandbox project and then the name is specified in the EntryPoint
Application* CreateApplication(const std::string& name);