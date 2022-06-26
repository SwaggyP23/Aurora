#include "../OGLpch.h"
#include "Window.h"

void error_callback(int error, const char* description)
{
	CORE_LOG_ERROR("GLFW Error ({0}): {1}", error, description);
}

Window::Window(const std::string& title, unsigned int width, unsigned int height)
{
	logger::Log::Init(); // Move this to application constructor once the entrypoint is created

	Init(title, width, height);
}

Window::~Window()
{
	ShutDown();
}

void Window::enable(GLenum type) const
{
	glEnable(type);
}

void Window::disable(GLenum type) const
{
	glEnable(type);
}

void Window::SetVSync(bool state)
{
	glfwSwapInterval(state);
	m_Data.VSync = state;
}

bool Window::closed() const
{
	return glfwWindowShouldClose(m_Window) == 1;
}

void Window::update() const
{
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
		CORE_LOG_ERROR("OpenGL Error: {0}, Function: {1}", error, __FUNCTION__);

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwPollEvents();
	glfwGetFramebufferSize(m_Window, (int*)&m_Data.Width, (int*)&m_Data.Height);
	glfwSwapBuffers(m_Window);
}

void Window::clear(float x, float y, float z, float w) const
{
	glClearColor(x, y, z, w);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();
}

bool Window::Init(const std::string& title, unsigned int width, unsigned int height)
{
	m_Data.Title = title;
	m_Data.Width = width;
	m_Data.Height = height;

	CORE_LOG_INFO("Creating window {0} ({1}, {2})", m_Data.Title, m_Data.Width, m_Data.Height);

	if (!glfwInit()) {
		CORE_LOG_ERROR("Failed to initialize glfw!");
		
		return false;
	}
	glfwSetErrorCallback(error_callback);

	m_Window = glfwCreateWindow(m_Data.Width, m_Data.Height, m_Data.Title.c_str(), NULL, NULL);

	if (!m_Window) {
		CORE_LOG_ERROR("Failed to initialize the window!");

		return false;
	}

	glfwMakeContextCurrent(m_Window);
	glfwSetWindowUserPointer(m_Window, &m_Data);

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

	glViewport(0, 0, m_Data.Width, m_Data.Height);

	if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		CORE_LOG_ERROR("Failed to initialize glad!!");

		return false;
	}

	CORE_LOG_INFO("OpenGL Version: {0}", (const char*)glGetString(GL_VERSION));

	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
	ImGui_ImplOpenGL3_Init();

	ImGuiIO& io = ImGui::GetIO();
	io.WantCaptureMouse = true;
	io.WantCaptureKeyboard = true;


	return true;
}

void Window::ShutDown()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(m_Window);
	glfwTerminate();
}