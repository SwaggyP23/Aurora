#pragma once

#include "Core/Base.h"
#include "Renderer/RendererCaps.h"

struct GLFWwindow;

namespace Aurora {

	class RenderContext : public RefCountedObject
	{
	public:
		RenderContext(GLFWwindow* windowHandle);
		virtual ~RenderContext() = default;

		static Ref<RenderContext> Create(GLFWwindow* handle);

		void Init();
		void Shutdown();

		void SwapBuffers() const;
		void PollEvents() const;

	private:
		GLFWwindow* m_WindowHandle = nullptr;

	};

}