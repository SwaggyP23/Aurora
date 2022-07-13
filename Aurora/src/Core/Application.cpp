#include "Aurorapch.h"
#include "Application.h"

#include "Renderer/Renderer.h"

namespace Aurora {

	Application* Application::s_Instance = nullptr;

	Application::Application(const std::string& name)
	{
		AR_PROFILE_FUNCTION();

		s_Instance = this;

		m_Window = Window::Create(name, 1280, 720);
		m_Window->SetEventCallback(AR_SET_EVENT_FN(Application::onEvent));

		Renderer::Init(); // This handles the Renderer3D initiation since im taking this as the general renderer

		m_ImGuiLayer = new ImGuiLayer();
		pushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{
		AR_PROFILE_FUNCTION();

		// Layers' onDetach() function is called from the LayerStack destructor due to RAII since the layerstack in application is on
		// the stack

		Renderer::ShutDown();
	}

	void Application::Close()
	{
		m_Running = false;
	}

	void Application::pushLayer(Layer* layer)
	{
		AR_PROFILE_FUNCTION();

		m_LayerStack.pushLayer(layer);
		layer->onAttach();
	}

	void Application::pushOverlay(Layer* layer)
	{
		AR_PROFILE_FUNCTION();

		m_LayerStack.pushOverlay(layer);
		layer->onAttach();
	}

	void Application::onEvent(Event& e)
	{
		AR_PROFILE_FUNCTION();

		m_Window->SetVSync(m_VSync);

		EventDispatcher dispatcher(e);
		dispatcher.dispatch<WindowCloseEvent>(AR_SET_EVENT_FN(Application::onWindowClose));
		dispatcher.dispatch<WindowResizeEvent>(AR_SET_EVENT_FN(Application::onWindowResize));
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
		AR_PROFILE_FUNCTION();

		while (m_Running) // Render Loop.
		{
			AR_PROFILE_SCOPE("Run Loop");

			float currentFrame = (float)(glfwGetTime());
			TimeStep timeStep = currentFrame - m_LastFrame;
			m_LastFrame = currentFrame;

			if (!m_Minimized)
			{
				{
					AR_PROFILE_SCOPE("LayerStack Updating!");

					for (Layer* layer : m_LayerStack)
						layer->onUpdate(timeStep);
				}

				m_ImGuiLayer->begin();
				{
					AR_PROFILE_SCOPE("LayerStack ImGui Rendering!");

					for (Layer* layer : m_LayerStack)
						layer->onImGuiRender();
				}
				m_ImGuiLayer->end();
			}

			m_Window->Update();
		}
	}

	bool Application::onWindowResize(WindowResizeEvent& e)
	{
		AR_PROFILE_FUNCTION();

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

}