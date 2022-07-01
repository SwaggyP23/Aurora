#include "OGLpch.h"
#include "Application.h"

#include "Graphics/Renderer.h"

Application* Application::s_Instance = nullptr;

Application::Application(const std::string& name)
{
	s_Instance = this;

	m_Window = std::make_shared<Window>(name, 1280, 720);
	m_Window->enable(GL_DEPTH_TEST);
	m_Window->SetVSync(true);
	m_Window->SetEventCallback(SET_EVENT_FN(Application::onEvent));

	std::vector<char> errorMessage;

	m_ImGuiLayer = new ImGuiLayer();
	pushOverlay(m_ImGuiLayer);
	//pushLayer(new SandBoxLayer);
}

Application::~Application()
{
}

void Application::pushLayer(Layer* layer)
{
	m_LayerStack.pushLayer(layer);
	layer->onAttach();
}

void Application::pushOverlay(Layer* layer)
{
	m_LayerStack.pushOverlay(layer);
	layer->onAttach();
}

void Application::onEvent(Event& e)
{
	m_Window->SetVSync(m_VSync);	

	EventDispatcher dispatcher(e);
	dispatcher.dispatch<WindowCloseEvent>(SET_EVENT_FN(Application::onWindowClose));
	dispatcher.dispatch<WindowResizeEvent>(SET_EVENT_FN(Application::onWindowResize));
	//LOG_INFO("{0}", e);

	for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
	{
		if (e.Handled)
			break;
		(*it)->onEvent(e);
	}
}

void Application::Run()
{
	while (m_Running) // Render Loop.
	{
		float currentFrame = (float)(glfwGetTime());
		TimeStep time = currentFrame - m_LastFrame;
		m_LastFrame = currentFrame;

		for (Layer* layer : m_LayerStack)
			layer->onUpdate(/*should take in timestep*/);

		m_ImGuiLayer->begin();
		for (Layer* layer : m_LayerStack)
			layer->onImGuiRender();
		m_ImGuiLayer->end();

		m_Window->update();
	}
}

bool Application::onWindowResize(WindowResizeEvent& e)
{
	Renderer::onWindowResize(e.getWidth(), e.getHeight());

	return true;
}

bool Application::onWindowClose(WindowCloseEvent& e)
{
	m_Running = false;
	return true;
}