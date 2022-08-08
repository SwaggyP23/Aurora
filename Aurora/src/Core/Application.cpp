#include "Aurorapch.h"
#include "Application.h"

#include "Renderer/Renderer3D.h"
#include "Utils/UtilFunctions.h"

extern bool g_ApplicationRunning;

namespace Aurora {

	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& specification)
	{
		AR_PROFILE_FUNCTION();

		s_Instance = this;

		if (!m_Specification.WorkingDirectory.empty())
			std::filesystem::current_path(m_Specification.WorkingDirectory);

		WindowSpecification windowSpec;
		windowSpec.Title = specification.Name;
		windowSpec.Width = specification.WindowWidth;
		windowSpec.Height = specification.WindowHeight;
		windowSpec.Decorated = specification.WindowDecorated;
		windowSpec.VSync = specification.VSync;
		windowSpec.Resizable = specification.SetWindowResizable;
		windowSpec.WindowIconPath = specification.ApplicationWindowIconPath;
		m_Window = Window::Create(windowSpec);
		m_Window->Init();
		m_Window->SetEventCallback(AR_SET_EVENT_FN(Application::OnEvent));
		if (specification.StartMaximized)
			m_Window->CreateMaximized();
		else
			m_Window->CreateCentred();

		Renderer3D::Init(); // This handles the Renderer3D, RenderCommand and RendererProperties initiation

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{
		AR_PROFILE_FUNCTION();

		m_Window->SetEventCallback([](Event& e) {});

		for (Layer* layer : m_LayerStack)
		{
			layer->OnDetach();
			delete layer;
		}

		Renderer3D::ShutDown(); // Look into moving to Aurora Core Shutdown with similar shutdown functions
	}

	void Application::PushLayer(Layer* layer)
	{
		AR_PROFILE_FUNCTION();

		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		AR_PROFILE_FUNCTION();

		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	void Application::PopLayer(Layer* layer)
	{
		AR_PROFILE_FUNCTION();

		m_LayerStack.PopLayer(layer);
		layer->OnDetach();
	}

	void Application::PopOverlay(Layer* layer)
	{
		AR_PROFILE_FUNCTION();

		m_LayerStack.PopOverlay(layer);
		layer->OnDetach();
	}

	void Application::RenderImGui()
	{
		AR_PROFILE_FUNCTION();

		m_ImGuiLayer->Begin();

		for (Layer* layer : m_LayerStack)
			layer->OnImGuiRender();

		m_ImGuiLayer->End();
	}

	void Application::Run()
	{
		OnInit();
		while (m_Running) // Render Loop.
		{
			AR_ENDF_TIMER(); // This is for the UI timers
			AR_PROFILE_FRAME("Game Loop");

			// static uint64_t frameCounter = 0;

			ProcessEvents();

			if (!m_Minimized)
			{
				Timer cpuTimer;
				{
					AR_PROFILE_SCOPE("Application Layer::OnUpdate");

					for (Layer* layer : m_LayerStack)
						layer->OnUpdate(m_Timestep);
				}

				RenderImGui(); // All the Perf timers dont work after this // TODO: Fix!

				m_CPUTime = cpuTimer.ElapsedMillis();
				m_Window->Update();
			}

			float time = Utils::Time::GetTime();
			m_FrameTime = time - (float)m_TimeSinceStart;
			m_Timestep = glm::min<float>(m_FrameTime, 0.0333f);
			m_TimeSinceStart = time;

			// frameCounter++; // This is to be displayed some time later when needed...
		}
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		AR_PROFILE_FUNCTION();

		if (e.GetWidth() == 0 || e.GetHeight() == 0) {
			m_Minimized = true;

			return false;
		}

		m_Minimized = false;

		Renderer3D::OnWindowResize(e.GetWidth(), e.GetHeight());

		return true; // This return is what sets the Handled bool in the event to true or false
	}

	bool Application::OnWindowMinimize(WindowMinimizeEvent& e)
	{
		m_Minimized = true;

		return true; // This return is what sets the Handled bool in the event to true or false
	}

	bool Application::OnWindowMaximize(WindowMaximizeEvent& e)
	{
		m_Minimized = false;
		m_Window->Maximize();

		return true; // This return is what sets the Handled bool in the event to true or false
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		g_ApplicationRunning = false;

		return true; // This return is what sets the Handled bool in the event to true or false
	}

	void Application::Close()
	{
		m_Running = false;
		g_ApplicationRunning = false;
	}

	void Application::Restart()
	{
		m_Running = false;
	}

	void Application::ProcessEvents()
	{
		m_Window->PollEvents(); // TODO: Add custom queue event handling...
	}

	void Application::OnEvent(Event& e)
	{
		AR_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.dispatch<WindowCloseEvent>(AR_SET_EVENT_FN(Application::OnWindowClose));
		dispatcher.dispatch<WindowMinimizeEvent>(AR_SET_EVENT_FN(Application::OnWindowMinimize));
		dispatcher.dispatch<WindowMaximizeEvent>(AR_SET_EVENT_FN(Application::OnWindowMaximize));
		dispatcher.dispatch<WindowResizeEvent>(AR_SET_EVENT_FN(Application::OnWindowResize));

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			(*it)->OnEvent(e);
			if (e.Handled)
				break;
		}
	}

}