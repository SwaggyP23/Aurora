#include "Aurorapch.h"
#include "Window.h"

#include "Core/Input/Input.h"
#include "Utils/ImageLoader.h"

#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <imgui/imgui.h>

namespace Aurora {

	static void MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
	{
		const char* ErrorType;
		switch (type)
		{
		    case GL_DEBUG_TYPE_ERROR:                   ErrorType = "ERROR"; break;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:     ErrorType = "DEPRECATED BEHAVIOR"; break;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:      ErrorType = "UNDEFINED_BEHAVIOR"; break;
			case GL_DEBUG_TYPE_PORTABILITY:             ErrorType = "PORTABILITY"; break;
			case GL_DEBUG_TYPE_PERFORMANCE:             ErrorType = "PERFORMANCE"; break;
			case GL_DEBUG_TYPE_OTHER:                   ErrorType = "OTHER"; break;
		}

		if (severity == GL_DEBUG_SEVERITY_HIGH)
			AR_CORE_CRITICAL_TAG("OpenGL Debug HIGH", "Type: {0}, Message : {1}", ErrorType, message);
		else if (severity == GL_DEBUG_SEVERITY_MEDIUM)
			AR_CORE_WARN_TAG("OpenGL Debug MEDIUM", "Type: {0}, Message : {1}", ErrorType, message);
	}

	static void glfw_error_callback(int error, const char* description)
	{
		AR_CORE_ERROR_TAG("Window", "GLFW Error({0}) : {1}", error, description);
	}

	Scope<Window> Window::Create(const WindowSpecification& spec)
	{
		return CreateScope<Window>(spec);
	}

	Window::Window(const WindowSpecification& spec)
		: m_Specification(spec)
	{
	}

	void Window::Init()
	{
		AR_PROFILE_FUNCTION();

		m_Data.Title = m_Specification.Title;
		m_Data.Width = m_Specification.Width;
		m_Data.Height = m_Specification.Height;

		glfwInitHint(GLFW_JOYSTICK_HAT_BUTTONS, GLFW_FALSE);

		int success = glfwInit();
		AR_CORE_ASSERT(success, "Failed to initialize glfw!");

#ifdef AURORA_DEBUG
		glfwSetErrorCallback(glfw_error_callback);
#endif

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_DECORATED, m_Specification.Decorated ? GLFW_TRUE : GLFW_FALSE);

		glfwWindowHint(GLFW_RESIZABLE, m_Specification.Resizable ? GLFW_TRUE : GLFW_FALSE);
	
		if (m_Specification.FullScreen)
		{
			CreateMaximized();
		}
		else
		{
			CreateCentred();
		}

		m_Context = RenderContext::Create(m_Window);
		m_Context->Init();

#ifdef AURORA_DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(MessageCallback, nullptr); // This is for OpenGL error callback and is the one that gets called
#endif

		glfwSetWindowUserPointer(m_Window, &m_Data);

		if (glfwRawMouseMotionSupported())
			glfwSetInputMode(m_Window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		else
			AR_CORE_WARN_TAG("Window", "Raw mouse motion is not supported!");

		SetGLFWCallbacks();

		SetIconImage();
		SetVSync(m_Specification.VSync); // A context needs to be current for this which is done in m_Context->Init();
	}

	Window::~Window()
	{
		ShutDown(); // Maybe this function and function like it should go into the Aurora Shutdown Core in the Initializers header

		AR_PROFILE_END_SESSION("ApplicationShutdown");
	}

	void Window::CreateMaximized()
	{
		AR_PROFILE_FUNCTION();

		GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

		AR_CORE_INFO_TAG("Window", "Creating Window: {0} ({1}, {2})", m_Specification.Title, mode->width, mode->height);

		glfwWindowHint(GLFW_DECORATED, false);
		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

		m_Window = glfwCreateWindow(mode->width, mode->height, m_Data.Title.c_str(), primaryMonitor, nullptr);
		AR_CORE_ASSERT(m_Window, "Failed to initialize the window!");
	}

	void Window::CreateCentred()
	{
		AR_PROFILE_FUNCTION();

		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);

		AR_CORE_INFO_TAG("Window", "Creating window: {0} ({1}, {2})", m_Specification.Title, m_Specification.Width, m_Specification.Height);

		m_Window = glfwCreateWindow(m_Data.Width, m_Data.Height, m_Data.Title.c_str(), NULL, NULL);
		AR_CORE_ASSERT(m_Window, "Failed to initialize the window!");

		glfwSetWindowPos(m_Window, (mode->width - m_Data.Width) / 2, (mode->height - m_Data.Height) / 2);
	}

	void Window::SetVSync(bool state)
	{
		glfwSwapInterval(state);
		m_Specification.VSync = state;
	}

	void Window::ProcessEvents() const
	{
		m_Context->PollEvents();
		Input::Update();
	}

	void Window::SwapBuffers() const
	{
#ifdef AURORA_DEBUG
		GLenum error = glGetError();
		while (error != GL_NO_ERROR)
		{
			AR_CORE_ERROR_TAG("Window", "OpenGL Error : {0}", error);
			error = glGetError();
		}
#endif

		m_Context->SwapBuffers();
	}

	void Window::Maximize() const
	{
		AR_PROFILE_FUNCTION();

		AR_CORE_INFO_TAG("Window", "Maximizing Window...");
		glfwMaximizeWindow(m_Window);
	}

	void Window::Center() const
	{
		AR_PROFILE_FUNCTION();

		AR_CORE_INFO_TAG("Window", "Centering Window...");
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);

		glfwSetWindowPos(m_Window, (mode->width - m_Data.Width) / 2, (mode->height - m_Data.Height) / 2);
	}

	void Window::SetTitle(const std::string& newName)
	{
		glfwSetWindowTitle(m_Window, newName.c_str());
	}

	void Window::SetIconImage()
	{
		AR_PROFILE_FUNCTION();

		Utils::ImageData imageData = Utils::ImageLoader::LoadImageFile(m_Specification.WindowIconPath);

		GLFWimage images[1];
		images[0].width = imageData.Width;
		images[0].height = imageData.Height;
		images[0].pixels = imageData.PixelData;

		glfwSetWindowIcon(m_Window, 1, images);

		Utils::ImageLoader::FreeImage(imageData.PixelData); // Free image right after glfw copies its data internally
	}

	void Window::SetGLFWCallbacks()
	{
		AR_PROFILE_FUNCTION();

		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			WindowResizeEvent event(width, height);
			data.EventCallback(event);
			data.Width = width;
			data.Height = height;
		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			WindowCloseEvent event;
			data.EventCallback(event);
		});

		glfwSetWindowIconifyCallback(m_Window, [](GLFWwindow* window, int iconified)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			WindowMinimizeEvent event((bool)iconified);
			data.EventCallback(event);
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
			    case GLFW_PRESS:
			    {
					Input::UpdateKeyState((KeyCode)key, KeyState::Pressed);
			    	KeyPressedEvent event((KeyCode)key, 0);
			    	data.EventCallback(event);
			    	break;
			    }
			    case GLFW_RELEASE:
			    {
					Input::UpdateKeyState((KeyCode)key, KeyState::Released);
			    	KeyReleasedEvent event((KeyCode)key);
			    	data.EventCallback(event);
			    	break;
			    }
			    case GLFW_REPEAT:
			    {
					Input::UpdateKeyState((KeyCode)key, KeyState::Held);
			    	KeyPressedEvent event((KeyCode)key, true);
			    	data.EventCallback(event);
			    	break;
			    }
			}
		});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			KeyTypedEvent event((KeyCode)keycode);
			data.EventCallback(event);
		});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
			case GLFW_PRESS:
			{
				MouseButtonPressedEvent event((MouseButton)button);
				data.EventCallback(event);
				break;
			}
			case GLFW_RELEASE:
			{
				MouseButtonReleasedEvent event((MouseButton)button);
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

		glfwSetDropCallback(m_Window, [](GLFWwindow* window, int path_count, const char* paths[])
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			WindowPathDropEvent event(path_count, paths);
			data.EventCallback(event);
		});

		m_ImGuiMouseCursors[ImGuiMouseCursor_Arrow]       = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		m_ImGuiMouseCursors[ImGuiMouseCursor_TextInput]   = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
		m_ImGuiMouseCursors[ImGuiMouseCursor_ResizeAll]   = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);   // FIXME: GLFW doesn't have this.
		m_ImGuiMouseCursors[ImGuiMouseCursor_ResizeNS]    = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
		m_ImGuiMouseCursors[ImGuiMouseCursor_ResizeEW]    = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
		m_ImGuiMouseCursors[ImGuiMouseCursor_ResizeNESW]  = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);   // FIXME: GLFW doesn't have this.
		m_ImGuiMouseCursors[ImGuiMouseCursor_ResizeNWSE]  = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);   // FIXME: GLFW doesn't have this.
		m_ImGuiMouseCursors[ImGuiMouseCursor_Hand]        = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

		glfwGetWindowSize(m_Window, (int*)& m_Data.Width, (int*)&m_Data.Height);
	}

	void Window::ShutDown()
	{
		AR_PROFILE_FUNCTION();

		glfwDestroyWindow(m_Window);
		m_Window = nullptr;

		m_Context->Shutdown();
	}

}