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
public:
	Window(const char* title, unsigned int width, unsigned int height, void(*func)(GLFWwindow*, double, double), void(*funcS)(GLFWwindow*, double, double));
	~Window();

	void enable(GLenum type) const;
	void disable(GLenum type) const;
	void SetVSync(bool var);
	bool closed() const;
	void update() const;
	void clear(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 1.0f) const;

	inline unsigned int getWidth() const { return m_Width; }
	inline unsigned int getHeight() const { return m_Height; }
	inline GLFWwindow* getWindowPointer() const { return m_Window; }

	bool isKeyPressed(unsigned int keycode) const;
	bool isMouseButtonPressed(unsigned int buttonCode) const;
	void getCursorPosition(double& x, double& y) const;
	void getScrollPosition(double& xoff, double& yoff) const;

private:
	bool Init();

	friend void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	friend void cursor_callback(GLFWwindow* window, double xpos, double ypos);
	friend void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	friend void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	friend void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
	GLFWwindow* m_Window;
	const char* m_Title;
	unsigned int m_Width, m_Height;
	bool m_Closed;
	void(*m_Function)(GLFWwindow*, double, double);
	void(*m_FunctionS)(GLFWwindow*, double, double);

	static double m_X, m_Y;
	static double m_Xoff, m_Yoff;
	static bool m_Keys[350];
	static bool m_MouseButtons[10];
};

#endif