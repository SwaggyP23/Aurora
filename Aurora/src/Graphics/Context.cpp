#include "Aurorapch.h"
#include "Context.h"

#include <glfw/glfw3.h>
#include <glad/glad.h>

namespace Aurora {

	Ref<Context> Context::Create(GLFWwindow* handle)
	{
		return CreateRef<Context>(handle);
	}

	Context::Context(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		AR_CORE_ASSERT(m_WindowHandle, "Window handle is null!");
	}

	void Context::Init() const
	{
		AR_PROFILE_FUNCTION();

		glfwMakeContextCurrent(m_WindowHandle);

		int gladSuccess = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		AR_CORE_ASSERT(gladSuccess, "Failed to initialize glad!");

		AR_CORE_ASSERT(GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 5), "OpenGL version is less that 4.5!");
	}

	void Context::SwapBuffers() const
	{
		AR_PROFILE_FUNCTION();

		glfwSwapBuffers(m_WindowHandle);
	}

	void Context::PollEvents() const
	{
		AR_PROFILE_FUNCTION();

		glfwPollEvents();
	}

}