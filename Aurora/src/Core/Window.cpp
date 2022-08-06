#include "Aurorapch.h"
#include "Window.h"

#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <imgui/imgui.h>

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

	Scope<Window> Window::Create(const WindowSpecification& spec)
	{
		return CreateScope<Window>(spec);
	}

	Window::Window(const WindowSpecification& spec)
		: m_Specification(spec)
	{
	}

	Window::~Window()
	{
		ShutDown(); // Maybe this function and function like it should go into the Aurora Shutdown Core in the Initializers header
	}

	void Window::SetVSync(bool state)
	{
		AR_PROFILE_FUNCTION();

		glfwSwapInterval(state);
		m_Specification.VSync = state;
	}

	void Window::PollEvents() const
	{
		m_Context->PollEvents();
	}

	void Window::Update() const
	{
		AR_PROFILE_FUNCTION();

#ifdef AURORA_DEBUG
		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
			AR_CORE_ERROR("OpenGL Error: {0}, Function: {1}", error, __FUNCTION__);
#endif

		m_Context->SwapBuffers();
	}

	void Window::Maximize() const
	{
		AR_PROFILE_FUNCTION();

		glfwMaximizeWindow(m_Window);
	}

	bool Window::Init()
	{
		AR_PROFILE_FUNCTION();

		AR_CORE_INFO("Creating window {0} ({1}, {2})", m_Specification.Title, m_Specification.Width, m_Specification.Height);

		int success = glfwInit();
		AR_CORE_ASSERT(success, "Failed to initialize glfw!");

#ifdef AURORA_DEBUG
		glfwSetErrorCallback(error_callback);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE); // This is for OpenGL error callback
#endif

		if (m_Specification.Decorated)
			glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
		else
			glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

		if (m_Specification.Resizable)
			glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		else
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		return true;
	}

	void Window::CreateMaximized()
	{
		AR_PROFILE_FUNCTION();

		m_Window = glfwCreateWindow(m_Specification.Width, m_Specification.Height, m_Specification.Title.c_str(), NULL, NULL);
		AR_CORE_ASSERT(m_Window, "Failed to initialize the window!");
		
		glfwMaximizeWindow(m_Window);

		m_Context = Context::Create(m_Window);
		m_Context->Init();		

		SetIconImage();
		SetVSync(m_Specification.VSync); // A context needs to be current for this which is done in m_Context->Init();

#ifdef AURORA_DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(MessageCallback, 0); // This is for GLFW error callback and is the one that gets called
#endif

		glfwSetWindowUserPointer(m_Window, &m_Specification);

		SetGLFWCallbacks();
	}

	void Window::CreateCentred()
	{
		AR_PROFILE_FUNCTION();

		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);

		m_Window = glfwCreateWindow(m_Specification.Width, m_Specification.Height, m_Specification.Title.c_str(), NULL, NULL);
		AR_CORE_ASSERT(m_Window, "Failed to initialize the window!");

		glfwSetWindowPos(m_Window, (mode->width - m_Specification.Width) / 2, (mode->height - m_Specification.Height) / 2);

		m_Context = Context::Create(m_Window);
		m_Context->Init();

		SetIconImage();
		SetVSync(m_Specification.VSync); // A context needs to be current for this which is done in m_Context->Init();

#ifdef AURORA_DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(MessageCallback, 0); // This is for GLFW error callback and is the one that gets called
#endif

		glfwSetWindowUserPointer(m_Window, &m_Specification);

		SetGLFWCallbacks();
	}

	void Window::SetIconImage()
	{
		AR_PROFILE_FUNCTION();

		auto& imageData = Utils::ImageLoader::LoadImageFile(m_Specification.WindowIconPath);

		GLFWimage images[1];
		images[0].width = imageData.Width;
		images[0].height = imageData.Height;
		images[0].pixels = imageData.PixelData;

		glfwSetWindowIcon(m_Window, 1, images);

		Utils::ImageLoader::FreeImage(); // Free image right after glfw copies its data internally
	}

	void Window::SetGLFWCallbacks()
	{
		AR_PROFILE_FUNCTION();

		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
		{
			WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.EventCallback(event);
		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);

			WindowCloseEvent event;
			data.EventCallback(event);
		});

		glfwSetWindowIconifyCallback(m_Window, [](GLFWwindow* window, int iconified)
		{
			WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);

			if (iconified)
			{
				WindowMinimizeEvent event;
				data.EventCallback(event);
			}
		});

		glfwSetWindowMaximizeCallback(m_Window, [](GLFWwindow* window, int maximized) 
		{
			WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);

			if (maximized)
			{
				WindowMaximizeEvent event;
				data.EventCallback(event);
			}
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);

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
			WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);

			KeyTypedEvent event(keycode);
			data.EventCallback(event);
		});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
		{
			WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);

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
			WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.EventCallback(event);
		});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
		{
			WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);

			MouseMovedEvent event((float)xPos, (float)yPos);
			data.EventCallback(event);
		});
	}

	void Window::ShutDown()
	{
		AR_PROFILE_FUNCTION();

		glfwDestroyWindow(m_Window);
		m_Window = nullptr;
		glfwTerminate();
	}

}