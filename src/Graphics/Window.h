#pragma once

#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <IMGUI/imgui.h>
#include <IMGUI/imgui_impl_glfw.h>
#include <IMGUI/imgui_impl_opengl3.h>

#include "Events/Events.h"

#include <functional>

// The reasom the app crashes now is because the callbacks for the events are not set so functional throws

class Window
{
	using EventCallbackFn = std::function<void(Event&)>;
public:
	Window(std::string title, unsigned int width, unsigned int height);
	~Window();

	void enable(GLenum type) const;
	void disable(GLenum type) const;

	inline void SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback; }

	void SetVSync(bool state);
	inline bool isVSync() const { return m_Data.VSync; }

	bool closed() const;
	void update() const;
	void clear(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 1.0f) const;

	inline unsigned int getWidth() const { return m_Data.Width; }
	inline unsigned int getHeight() const { return m_Data.Height; }
	inline GLFWwindow* getWindowPointer() const { return m_Window; }

private:
	bool Init(std::string title, unsigned int width, unsigned int height);
	void ShutDown();

	friend void framebuffer_size_callback(GLFWwindow* window, int width, int height);

private:
	GLFWwindow* m_Window;
	bool m_Closed;

	struct WindowData
	{
		std::string Title;
		unsigned int Width, Height;
		bool VSync;

		EventCallbackFn EventCallback;
	};

	WindowData m_Data;
};

#endif