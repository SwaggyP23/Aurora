#pragma once

#include "Events/ApplicationEvents.h"
#include "Events/MouseEvents.h"
#include "Events/KeyEvents.h"
#include "Graphics/Context.h"

#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <imgui/imgui.h>
#include <functional>

struct GLFWwindow;

namespace Aurora {

	struct WindowSpecification
	{
		std::string Title = "Aurora Window";
		uint32_t Width = 1280;
		uint32_t Height = 720;
		bool VSync = true;
		bool Resizable = false;
		bool Decorated = true;

		std::function<void(Event&)> EventCallback;
	};

	class Window
	{
		using EventCallbackFn = std::function<void(Event&)>;

	public:
		Window(const WindowSpecification& spec);
		~Window();

		static Scope<Window> Create(const WindowSpecification& spec);

		bool Init();

		void SetEventCallback(const EventCallbackFn& callback) { m_Specification.EventCallback = callback; }

		void Maximize();
		void CentreWindow();

		void SetVSync(bool state);
		inline bool IsVSync() const { return m_Specification.VSync; }

		void PollEvents() const;
		void Update() const;

		inline uint32_t GetWidth() const { return m_Specification.Width; }
		inline uint32_t GetHeight() const { return m_Specification.Height; }
		inline GLFWwindow* GetWindowPointer() const { return m_Window; }

	private:
		void SetGLFWCallbacks();
		void ShutDown();

	private:
		GLFWwindow* m_Window;
		Scope<Context> m_Context;

		WindowSpecification m_Specification;
	};

}