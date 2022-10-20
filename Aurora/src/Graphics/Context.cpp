#include "Aurorapch.h"
#include "Context.h"

#include <glfw/glfw3.h>
#include <glad/glad.h>

namespace Aurora {

	Ref<RenderContext> RenderContext::Create(GLFWwindow* handle)
	{
		return CreateRef<RenderContext>(handle);
	}

	RenderContext::RenderContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		AR_CORE_ASSERT(m_WindowHandle, "Window handle is null!");
	}

	void RenderContext::Init()
	{
		glfwMakeContextCurrent(m_WindowHandle);

		int gladSuccess = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		AR_CORE_ASSERT(gladSuccess, "Failed to initialize glad!");

		AR_CORE_ASSERT(GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 5), "OpenGL version is less that 4.5!");
	}

	void RenderContext::Shutdown()
	{
		glfwTerminate();
	}

	void RenderContext::SwapBuffers() const
	{
		glfwSwapBuffers(m_WindowHandle);
	}

	void RenderContext::PollEvents() const
	{
		glfwPollEvents();
	}

}