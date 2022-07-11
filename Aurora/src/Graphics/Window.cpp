#include "Aurorapch.h"
#include "Window.h"

namespace Aurora {

	static void error_callback(int error, const char* description)
	{
		CORE_LOG_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	Ref<Window> Window::Create(const std::string& title, uint32_t width, uint32_t height)
	{
		return CreateRef<Window>(title, width, height);
	}

	Window::Window(const std::string& title, uint32_t width, uint32_t height)
	{
		PROFILE_FUNCTION();

		Init(title, width, height);
	}

	Window::~Window()
	{
		PROFILE_FUNCTION();

		ShutDown();
	}

	void Window::SetVSync(bool state)
	{
		PROFILE_FUNCTION();

		glfwSwapInterval(state);
		m_Data.VSync = state;
	}

	void Window::Update() const
	{
#ifdef _DEBUG
		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
			CORE_LOG_ERROR("OpenGL Error: {0}, Function: {1}", error, __FUNCTION__);
#endif

		PROFILE_FUNCTION();

		glfwPollEvents();
		//glfwGetFramebufferSize(m_Window, (int*)&m_Data.Width, (int*)&m_Data.Height);
		m_Context->SwapBuffers();
	}

	bool Window::Init(const std::string& title, uint32_t width, uint32_t height)
	{
		// PROFILE_FUNCTION(); Currently not needed since the constructor just calls Init();

		m_Data.Title = title;
		m_Data.Width = width;
		m_Data.Height = height;

		CORE_LOG_INFO("Creating window {0} ({1}, {2})", m_Data.Title, m_Data.Width, m_Data.Height);

		int success = glfwInit();
		CORE_ASSERT(success, "Failed to initialize glfw!");

		glfwSetErrorCallback(error_callback);
#ifdef _DEBUG
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

		m_Window = glfwCreateWindow(m_Data.Width, m_Data.Height, m_Data.Title.c_str(), NULL, NULL);
		CORE_ASSERT(m_Window, "Failed to initialize the window!");

		m_Context = Context::Create(m_Window);
		m_Context->Init();

		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(true);

		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				data.Width = width;
				data.Height = height;

				WindowResizeEvent event(width, height);
				data.EventCallback(event);
			});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				WindowCloseEvent event;
				data.EventCallback(event);
			});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				switch (action)
				{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(key, 0);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(key);
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, true);
					data.EventCallback(event);
					break;
				}
				}
			});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				KeyTypedEvent event(keycode);
				data.EventCallback(event);
			});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				switch (action)
				{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(button);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(button);
					data.EventCallback(event);
					break;
				}
				}
			});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				MouseScrolledEvent event((float)xOffset, (float)yOffset);
				data.EventCallback(event);
			});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				MouseMovedEvent event((float)xPos, (float)yPos);
				data.EventCallback(event);
			});

		return true;
	}

	void Window::ShutDown()
	{
		// PROFILE_FUNCTION(); Currently not needed since the destructor just calls ShutDown();

		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

}