#pragma once

#ifndef _WINDOW_H_
#define _WINDOW_H_

#include "OGLpch.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>

#include "Events/ApplicationEvents.h"
#include "Events/MouseEvents.h"
#include "Events/KeyEvents.h"

#include "Context.h"

class Window
{
	using EventCallbackFn = std::function<void(Event&)>;
public:
	Window(const std::string& title, uint32_t width, uint32_t height);
	~Window();

	static Ref<Window> Create(const std::string& title, uint32_t width, uint32_t height);

	inline void SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback; }

	void SetVSync(bool state);
	inline bool isVSync() const { return m_Data.VSync; }

	bool closed() const;
	void update() const;

	inline uint32_t getWidth() const { return m_Data.Width; }
	inline uint32_t getHeight() const { return m_Data.Height; }
	inline GLFWwindow* getWindowPointer() const { return m_Window; }

private:
	bool Init(const std::string& title, uint32_t width, uint32_t height);
	void ShutDown();

private:
	GLFWwindow* m_Window;
	Scope<Context> m_Context;

	struct WindowData
	{
		std::string Title;
		uint32_t Width, Height;
		bool VSync;

		EventCallbackFn EventCallback;
	};

	WindowData m_Data;
};

#endif