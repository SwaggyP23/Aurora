#include "Aurorapch.h"
#include "Context.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Aurora {

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
		PROFILE_FUNCTION();

		glfwMakeContextCurrent(m_WindowHandle);

		int gladSuccess = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		CORE_ASSERT(gladSuccess, "Failed to initialize glad!");

		CORE_ASSERT(GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 5), "OpenGL version is less that 4.5!");
	}

	void Context::SwapBuffers()
	{
		PROFILE_FUNCTION();

		glfwSwapBuffers(m_WindowHandle);
	}

}