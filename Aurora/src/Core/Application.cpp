#include "Aurorapch.h"
#include "Application.h"

#include "Input/Input.h"
#include "Renderer/Renderer.h"
#include "Utils/UtilFunctions.h"

extern bool g_ApplicationRunning;

namespace Aurora {

	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& specification)
		: m_Specification(specification)
	{
		AR_PROFILE_BEGIN_SESSION("ApplicationStartup", "Profiling");
		AR_PROFILE_FUNCTION();

		s_Instance = this;

		if (!m_Specification.WorkingDirectory.empty())
			std::filesystem::current_path(m_Specification.WorkingDirectory);

		m_Profiler = new PerformanceProfiler();

		WindowSpecification windowSpec = {};
		windowSpec.Title = specification.Name;
		windowSpec.Width = specification.WindowWidth;
		windowSpec.Height = specification.WindowHeight;
		windowSpec.FullScreen = specification.Fullscreen;
		windowSpec.Decorated = specification.WindowDecorated;
		windowSpec.VSync = specification.VSync;
		windowSpec.Resizable = specification.SetWindowResizable;
		windowSpec.WindowIconPath = specification.ApplicationWindowIconPath;
		m_Window = Window::Create(windowSpec);

		m_Window->Init();
		m_Window->SetEventCallback([this](Event& e) {Application::OnEvent(e); });
		if (specification.StartMaximized)
			m_Window->Maximize();
		else
			m_Window->Center();

		Renderer::Init(specification.RendererConfiguration);

		if (m_Specification.EnableImGui)
		{
			m_ImGuiLayer = new ImGuiLayer();
			PushOverlay(m_ImGuiLayer);
		}

		AR_PROFILE_END_SESSION("ApplicationStartup");
	}

	Application::~Application()
	{
		AR_PROFILE_BEGIN_SESSION("ApplicationShutdown", "Profiling");
		AR_PROFILE_FUNCTION();

		m_Window->SetEventCallback([](Event& e) {});

		for (Layer* layer : m_LayerStack)
		{
			layer->OnDetach();
			delete layer;
		}

		Renderer::ShutDown();

		delete m_Profiler;
		m_Profiler = nullptr;
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
		AR_SCOPE_PERF("Application::RenderImGui");

		m_ImGuiLayer->Begin();

		for (Layer* layer : m_LayerStack)
			layer->OnImGuiRender();

		m_ImGuiLayer->End();

		m_Profiler->Clear();
	}

	void Application::Run()
	{
		AR_PROFILE_BEGIN_SESSION("ApplicationRuntime", "Profiling");

		OnInit();

		long double timer = 0.0f;
		Timer TickTimer;
		while (m_Running) // Render Loop.
		{
			AR_PROFILE_FRAME("Game Loop");

			// static uint64_t frameCounter = 0;

			ProcessEvents();

			if (!m_Minimized)
			{
				Timer cpuTimer;

				// Updating the layers
				{
					AR_PROFILE_SCOPE("Application Layer::OnUpdate");
					AR_SCOPE_PERF("Application Layer::OnUpdate");

					for (Layer* layer : m_LayerStack)
						layer->OnUpdate(m_Timestep);
				}

				// Ticking the layers per second
				{
					AR_PROFILE_SCOPE("Application Layer::OnTick");
					AR_SCOPE_PERF("Application Layer::OnTick");

					if (TickTimer.Elapsed() - timer > m_TickDelta)
					{
						timer += m_TickDelta;

						for (Layer* layer : m_LayerStack)
							layer->OnTick();
					}
				}

				if(m_Specification.EnableImGui)
					RenderImGui();

				m_CPUTime = cpuTimer.ElapsedMillis();
				m_Window->SwapBuffers();
			}

			Input::ClearReleasedKeys();

			float time = Utils::Time::GetTime();
			m_FrameTime = time - m_LastFrameTime;
			m_Timestep = glm::min<float>(m_FrameTime, 0.0333f);
			m_LastFrameTime = time;

			// frameCounter++; // This is to be displayed some time later when needed...
		}
		OnShutdown();

		AR_PROFILE_END_SESSION("ApplicationRuntime");
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		// Since a window of 0, 0 width heigh is the same as if it minimized
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;

			return true;
		}

		m_Minimized = false;

		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}

	bool Application::OnWindowMinimize(WindowMinimizeEvent& e)
	{
		m_Minimized = e.IsMinimized();

		return false;
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		g_ApplicationRunning = false;

		return false;
	}

	void Application::Close()
	{
		m_Running = false;
	}

	void Application::OnShutdown()
	{
		m_EventCallbacks.clear();
		g_ApplicationRunning = false;
	}

	void Application::OnEvent(Event& e)
	{
		AR_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& e) { return Application::OnWindowClose(e); });
		dispatcher.Dispatch<WindowMinimizeEvent>([this](WindowMinimizeEvent& e) {return Application::OnWindowMinimize(e); });
		dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) {return Application::OnWindowResize(e); });

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			(*it)->OnEvent(e);
			if (e.Handled)
				break;
		}

		if (e.Handled)
			return;

		for (auto& eventCallback : m_EventCallbacks)
		{
			eventCallback(e);

			if (e.Handled)
				break;
		}
	}

	void Application::ProcessEvents()
	{
		Input::TransitionPressedKeys();

		m_Window->ProcessEvents();

		std::scoped_lock<std::mutex> lock(m_EventQueueMutex);

		while (m_EventQueue.size())
		{
			auto& func = m_EventQueue.front();
			func();
			m_EventQueue.pop();
		}
	}

}