#include "Aurorapch.h"
#include "Window.h"

namespace Aurora {

	static void MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
	{
		const char* ErrorType;
		switch (type)
		{
			case 0x824C:    ErrorType = "ERROR";
				break;
			case 0x824D:    ErrorType = "DEPRECATED BEHAVIOR";
				break;
			case 0x824E:    ErrorType = "UNDEFINED_BEHAVIOR";
				break;
			case 0x824F:    ErrorType = "PORTABILITY";
				break;
			case 0x8250:    ErrorType = "PERFORMANCE";
				break;
			case 0x8251:    ErrorType = "OTHER";
				break;
		}

		const char* Severity;
		switch (severity)
		{
			case 0x9146:	Severity = "HIGH";
				break;
			case 0x9147:	Severity = "MEIDUM";
				break;
		}
		
		if (severity == 0x9146)
			AR_CORE_CRITICAL("GL Callback: Type: {0}, Severity: {1}, Message: {2}", ErrorType, Severity, message);
		else if (severity == 0x9147)
			AR_CORE_WARN("GL Callback: Type: {0}, Severity: {1}, Message: {2}", ErrorType, Severity, message);
	}

	static void error_callback(int error, const char* description)
	{
		AR_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	Ref<Window> Window::Create(const std::string& title, uint32_t width, uint32_t height)
	{
		return CreateRef<Window>(title, width, height);
	}

	Window::Window(const std::string& title, uint32_t width, uint32_t height)
	{
		AR_PROFILE_FUNCTION();

		Init(title, width, height);
	}

	Window::~Window()
	{
		AR_PROFILE_FUNCTION();

		ShutDown();
	}

	void Window::SetVSync(bool state)
	{
		AR_PROFILE_FUNCTION();

		glfwSwapInterval(state);
		m_Data.VSync = state;
	}

	void Window::Update() const
	{
#ifdef AURORA_DEBUG
		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
			AR_CORE_ERROR("OpenGL Error: {0}, Function: {1}", error, __FUNCTION__);
#endif

		AR_PROFILE_FUNCTION();

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

		AR_CORE_INFO("Creating window {0} ({1}, {2})", m_Data.Title, m_Data.Width, m_Data.Height);

		int success = glfwInit();
		AR_CORE_ASSERT(success, "Failed to initialize glfw!");

#ifdef AURORA_DEBUG
		glfwSetErrorCallback(error_callback);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE); // This is for OpenGL error callback
#endif

		m_Window = glfwCreateWindow(m_Data.Width, m_Data.Height, m_Data.Title.c_str(), NULL, NULL);
		AR_CORE_ASSERT(m_Window, "Failed to initialize the window!");

		m_Context = Context::Create(m_Window);
		m_Context->Init();

#ifdef AURORA_DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(MessageCallback, 0); // This is for GLFW error callback and is the one that gets called
#endif

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