#include "OGLpch.h"
#include "Application.h"

#include "Graphics/Renderer.h"

Application* Application::s_Instance = nullptr;

Application::Application(const std::string& name)
{
	s_Instance = this;

	m_Window = Window::Create(name, 1280, 720);
	m_Window->SetEventCallback(SET_EVENT_FN(Application::onEvent));

	Renderer::Init(); // This handles the Renderer3D initiation since im taking this as the general renderer

	m_ImGuiLayer = new ImGuiLayer();
	pushOverlay(m_ImGuiLayer);
}

Application::~Application()
{
	for (Layer* layer : m_LayerStack) // This should be done so that each layer can free its resources in the onDetach function.
		layer->onDetach();			  // This way no memory leaks are supposed to happen from the application itself.

	Renderer::ShutDown();
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
		TimeStep timeStep = currentFrame - m_LastFrame;
		m_LastFrame = currentFrame;

		if (!m_Minimized)
		{
			for (Layer* layer : m_LayerStack)
				layer->onUpdate(timeStep);

			m_ImGuiLayer->begin();
			for (Layer* layer : m_LayerStack)
				layer->onImGuiRender();
			m_ImGuiLayer->end();
		}


		m_Window->update();
	}
}

bool Application::onWindowResize(WindowResizeEvent& e)
{
	if (e.getWidth() == 0 || e.getHeight() == 0) {
		m_Minimized = true;

		return false;
	}
	
	m_Minimized = false;

	Renderer::onWindowResize(e.getWidth(), e.getHeight());

	return false;
}

bool Application::onWindowClose(WindowCloseEvent& e)
{
	m_Running = false;
	return true;
}