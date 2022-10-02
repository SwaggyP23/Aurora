#pragma once

#include "Core/Base.h"
#include "Renderer/RendererCaps.h"

struct GLFWwindow;

namespace Aurora {

	class Context : public RefCountedObject
	{
	public:
		Context(GLFWwindow* windowHandle);
		virtual ~Context() = default;

		static Ref<Context> Create(GLFWwindow* handle);

		void Init();
		void Shutdown();

		void SwapBuffers() const;
		void PollEvents() const;

	private:
		GLFWwindow* m_WindowHandle = nullptr;

	};

}