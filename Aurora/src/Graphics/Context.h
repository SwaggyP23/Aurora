#pragma once

#include "Core/Base.h"
#include "Renderer/RendererPorperties.h"

struct GLFWwindow; // This is so that we dont have to include the glfw header because this will be included in window!

namespace Aurora {

	class Context
	{
	public:
		Context(GLFWwindow* windowHandle);
		static Scope<Context> Create(GLFWwindow* handle);

		void Init();
		void SwapBuffers();

	private:
		GLFWwindow* m_WindowHandle;

	};

}