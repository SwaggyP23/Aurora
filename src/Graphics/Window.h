#pragma once

#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <IMGUI/imgui.h>
#include <IMGUI/imgui_impl_glfw.h>
#include <IMGUI/imgui_impl_opengl3.h>

class Window
{
private:
	GLFWwindow* m_Window;
	const char* m_Title;
	unsigned int m_Width, m_Height;
	bool m_Closed;

	static double m_X, m_Y;

public:
	Window(const char* title, unsigned int width, unsigned int height);
	~Window();

	void enable(GLenum type) const;
	void disable(GLenum type) const;
	bool closed() const;
	void update() const;
	void clear(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 1.0f) const;

	inline unsigned int getWidth() const { return m_Width; }
	inline unsigned int getHeight() const { return m_Height; }
	inline GLFWwindow* getWindowPointer() const { return m_Window; }

	void getCursorPosition(double& x, double& y) const;

private:
	bool Init();

	friend void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	friend void cursor_callback(GLFWwindow* window, double xpos, double ypos);
};

#endif