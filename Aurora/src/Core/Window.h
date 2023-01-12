#pragma once

#include "Base.h"
#include "Events/ApplicationEvents.h"
#include "Events/MouseEvents.h"
#include "Events/KeyEvents.h"
#include "Graphics/Context.h"

#include <functional>

struct GLFWwindow;
struct GLFWcursor;

namespace Aurora {

	struct WindowSpecification
	{
		std::string Title = "Aurora Window";
		uint32_t Width = 1280;
		uint32_t Height = 720;
		bool VSync = true;
		bool FullScreen = false;
		bool Decorated = true;
		bool Resizable = true;

		std::string WindowIconPath;
	};

	class Window : public RefCountedObject
	{
		using EventCallbackFn = std::function<void(Event&)>;

	public:
		Window(const WindowSpecification& spec);
		virtual ~Window();

		[[nodiscard]] static Scope<Window> Create(const WindowSpecification& spec);

		void Init();

		void SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback; }

		void CreateMaximized();
		void CreateCentred();
		void Maximize() const;
		void Center() const;

		void SetTitle(const std::string& newName);

		void SetVSync(bool state);
		[[nodiscard]] inline bool IsVSync() const { return m_Specification.VSync; }

		void ProcessEvents() const;
		void SwapBuffers() const;

		[[nodiscard]] inline uint32_t GetWidth() const { return m_Data.Width; }
		[[nodiscard]] inline uint32_t GetHeight() const { return m_Data.Height; }
		[[nodiscard]] inline std::pair<uint32_t, uint32_t> GetSize() const { return { m_Data.Width, m_Data.Height }; }
		[[nodiscard]] inline GLFWwindow* GetWindowPointer() const { return m_Window; }

		[[nodiscard]] inline Ref<RenderContext> GetRenderContext() const { return m_Context; }

	private:
		void SetIconImage();
		void SetGLFWCallbacks();
		void ShutDown();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

	private:
		GLFWwindow* m_Window;
		GLFWcursor* m_ImGuiMouseCursors[9] = { 0 };
		WindowSpecification m_Specification;

		struct WindowData
		{
			std::string Title;
			uint32_t Width;
			uint32_t Height;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;

		Ref<RenderContext> m_Context;

	};

}