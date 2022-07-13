#pragma once

#include "Core/Base.h"
#include "Renderer/RendererPorperties.h"

struct GLFWwindow;

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