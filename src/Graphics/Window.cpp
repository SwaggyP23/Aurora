#include "Window.h"
#include "Logging/Log.h"

double Window::m_X, Window::m_Y;
bool Window::m_Keys[350];
bool Window::m_MouseButtons[10];

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void cursor_callback(GLFWwindow* window, double xpos, double ypos)
{
	Window* win = (Window*)glfwGetWindowUserPointer(window);
	win->m_X = xpos;
	win->m_Y = ypos;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Window* win = (Window*)glfwGetWindowUserPointer(window);
	win->m_Keys[key] = action != GLFW_RELEASE;
}

Window::Window(const char* title, unsigned int width, unsigned int height)
	: m_Width(width), m_Height(height), m_Title(title), m_Closed(false)
{
	logger::Log::Init();

	if (!Init())
		glfwTerminate();

	for (int i = 0; i < 350; i++)
		m_Keys[i] = false;

	for (int i = 0; i < 10; i++)
		m_MouseButtons[i] = false;
}

Window::~Window()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

void Window::enable(GLenum type) const
{
	glEnable(type);
}

void Window::disable(GLenum type) const
{
	glEnable(type);
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
	glfwGetFramebufferSize(m_Window, (int*)&m_Width, (int*)&m_Height);
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

bool Window::isKeyPressed(unsigned int keycode) const
{
	if (keycode >= 350) {
		CORE_LOG_ERROR("The keycode for the pressed key is greater than the max keycode!");

		return false;
	}

	return m_Keys[keycode];
}

bool Window::isMouseButtonPressed(unsigned int buttonCode) const
{
	if (buttonCode >= 10) {
		CORE_LOG_ERROR("The keycode for the pressed button is greater than the max keycode!");

		return false;
	}

	return m_MouseButtons[buttonCode];
}

void Window::getCursorPosition(double& x, double& y) const
{
	x = m_X;
	y = m_Y;
}

bool Window::Init()
{
	if (!glfwInit()) {
		CORE_LOG_ERROR("Failed to initialize glfw!");
		
		return false;
	}

	m_Window = glfwCreateWindow(m_Width, m_Height, m_Title, NULL, NULL);

	if (!m_Window) {
		CORE_LOG_ERROR("Failed to initialize the window!");

		return false;
	}

	glfwMakeContextCurrent(m_Window);
	glfwSetWindowUserPointer(m_Window, this);
	glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);
	glfwSetKeyCallback(m_Window, key_callback);
	glfwSetCursorPosCallback(m_Window, cursor_callback);
	glViewport(0, 0, m_Width, m_Height);

	if (glewInit() != GLEW_OK) {
		CORE_LOG_ERROR("Failed to initialize glew!");

		return false;
	}

	CORE_LOG_INFO("OpenGL Version: {0}", (const char*)glGetString(GL_VERSION));

	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
	ImGui_ImplOpenGL3_Init();

	return true;
}