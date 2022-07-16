#include "Aurorapch.h"
#include "Application.h"

#include "Debugging/ChromeInstrumentor.h"

#include "Renderer/Renderer.h"

namespace Aurora {

	Application* Application::s_Instance = nullptr;

	Application::Application(const std::string& name)
	{
		AR_OP_PROF_FUNCTION();

		s_Instance = this;

		m_Window = Window::Create(name, 1280, 720);
		m_Window->SetEventCallback(AR_SET_EVENT_FN(Application::onEvent));

		Renderer::Init(); // This handles the Renderer3D initiation since im taking this as the general renderer
		Aurora::Utils::Random::Init(); // This initializes the random number generator engine

		m_ImGuiLayer = new ImGuiLayer();
		pushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{
		AR_OP_PROF_FUNCTION();

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
		AR_OP_PROF_FUNCTION();

		m_LayerStack.pushLayer(layer);
		layer->OnAttach();
	}

	void Application::pushOverlay(Layer* layer)
	{
		AR_OP_PROF_FUNCTION();

		m_LayerStack.pushOverlay(layer);
		layer->OnAttach();
	}

	void Application::onEvent(Event& e)
	{
		AR_OP_PROF_FUNCTION();

		m_Window->SetVSync(m_VSync);

		EventDispatcher dispatcher(e);
		dispatcher.dispatch<WindowCloseEvent>(AR_SET_EVENT_FN(Application::onWindowClose));
		dispatcher.dispatch<WindowResizeEvent>(AR_SET_EVENT_FN(Application::onWindowResize));
		//LOG_INFO("{0}", e);

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			if (e.Handled)
				break;
			(*it)->OnEvent(e);
		}
	}

	void Application::Run()
	{
		while (m_Running) // Render Loop.
		{
			AR_ENDF_TIMER(); // This is for the imgui timers
			AR_OP_PROF_FRAME("Game Loop");

			float currentFrame = (float)(glfwGetTime());
			TimeStep timeStep = currentFrame - m_LastFrame;
			m_LastFrame = currentFrame;

			if (!m_Minimized)
			{
				{
					AR_OP_PROF_SCOPE_DYNAMIC("LayerStack Updating!");

					for (Layer* layer : m_LayerStack)
						layer->OnUpdate(timeStep);
				}

				m_ImGuiLayer->begin();
				{
					for (Layer* layer : m_LayerStack)
						layer->OnImGuiRender();
				}
				m_ImGuiLayer->end();
			}

			m_Window->Update();
		}
	}

	bool Application::onWindowResize(WindowResizeEvent& e)
	{
		AR_OP_PROF_FUNCTION();

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