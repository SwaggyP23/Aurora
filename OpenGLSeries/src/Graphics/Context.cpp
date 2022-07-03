#include "OGLpch.h"
#include "Context.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

Scope<Context> Context::Create(GLFWwindow* handle)
{
	return CreateScope<Context>(handle);
}

Context::Context(GLFWwindow* windowHandle)
	: m_WindowHandle(windowHandle)
{
	CORE_ASSERT(m_WindowHandle, "Window handle is null!");
}

void Context::Init()
{
	glfwMakeContextCurrent(m_WindowHandle);

	int gladSuccess = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	CORE_ASSERT(gladSuccess, "Failed to initialize glad!");

	CORE_LOG_INFO("OpenGL Info:");
	CORE_LOG_INFO("   --> Vendor: {0}", (const char*)glGetString(GL_VENDOR));
	CORE_LOG_INFO("   --> Renderer: {0}", (const char*)glGetString(GL_RENDERER));
	CORE_LOG_INFO("   --> Version: {0}", (const char*)glGetString(GL_VERSION));

	CORE_ASSERT(GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 5), "OpenGL version is less that 4.5!");
}

void Context::SwapBuffers()
{
	glfwSwapBuffers(m_WindowHandle);
}