#pragma once

#include "Core/Base.h"
#include "Renderer/RendererPorperties.h"

struct GLFWwindow;

namespace Aurora {

	class Context : public RefCountedObject
	{
	public:
		Context(GLFWwindow* windowHandle);
		static Ref<Context> Create(GLFWwindow* handle);

		void Init() const;
		void SwapBuffers() const;
		void PollEvents() const;

	private:
		GLFWwindow* m_WindowHandle;

	};

}